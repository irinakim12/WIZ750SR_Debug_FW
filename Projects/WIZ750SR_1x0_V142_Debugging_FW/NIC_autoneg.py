# nic_autoneg_windows.py
import os
import sys
import time
import ctypes
import subprocess


def run_cmd(cmd):
    try:
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            encoding="mbcs",      # Windows 한글 NIC 이름 대응
            errors="replace"
        )
        return result.stdout.strip(), result.stderr.strip(), result.returncode
    except Exception as e:
        return "", str(e), -1


def log(msg, level="INFO"):
    ts = time.strftime("%H:%M:%S")
    prefix = {
        "INFO": "[*]",
        "OK": "[+]",
        "WARN": "[W]",
        "ERR": "[!]"
    }.get(level, "[?]")
    print(f"{ts} {prefix} {msg}")


def check_admin():
    return ctypes.windll.shell32.IsUserAnAdmin() != 0


def ps_escape(s):
    return s.replace("'", "''")


class WindowsNIC:
    def list_interfaces(self):
        cmd = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            '"Get-NetAdapter | Select-Object -ExpandProperty Name"'
        )
        out, err, rc = run_cmd(cmd)

        if rc != 0:
            log(f"NIC 목록 조회 실패: {err}", "ERR")
            return []

        return [line.strip() for line in out.splitlines() if line.strip()]

    def get_status(self, nic_name):
        name = ps_escape(nic_name)
        cmd = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            f'"(Get-NetAdapter -Name \'{name}\').Status"'
        )
        out, err, rc = run_cmd(cmd)
        return out.strip(), err, rc

    def get_link_speed(self, nic_name):
        name = ps_escape(nic_name)
        cmd = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            f'"(Get-NetAdapter -Name \'{name}\').LinkSpeed"'
        )
        out, err, rc = run_cmd(cmd)
        return out.strip()

    def show_adapter_info(self, nic_name):
        name = ps_escape(nic_name)
        cmd = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            f'"Get-NetAdapter -Name \'{name}\' | '
            'Select-Object Name, InterfaceDescription, Status, LinkSpeed, MacAddress | '
            'Format-List"'
        )
        out, err, rc = run_cmd(cmd)

        if out:
            print(out)
        if rc != 0:
            log(f"상태 조회 실패: {err}", "ERR")

    def show_advanced_properties(self, nic_name):
        name = ps_escape(nic_name)
        cmd = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            f'"Get-NetAdapterAdvancedProperty -Name \'{name}\' | '
            'Where-Object { $_.RegistryKeyword -match \'Speed|Duplex|Auto|Link\' -or '
            '$_.DisplayName -match \'Speed|Duplex|Auto|Link\' } | '
            'Select-Object DisplayName, DisplayValue, RegistryKeyword, RegistryValue | '
            'Format-Table -AutoSize"'
        )
        out, err, rc = run_cmd(cmd)

        if out:
            print(out)
        if rc != 0:
            log(f"고급 속성 조회 실패: {err}", "WARN")

    def set_auto_negotiation(self, nic_name):
        name = ps_escape(nic_name)
        log(f"NIC '{nic_name}' Auto-Negotiation 설정 중...")

        candidates = [
            ("SpeedDuplex", "0"),
            ("AutoNegAdvertised", "63"),
        ]

        for keyword, value in candidates:
            cmd = (
                'powershell -NoProfile -ExecutionPolicy Bypass -Command '
                f'"Set-NetAdapterAdvancedProperty '
                f'-Name \'{name}\' '
                f'-RegistryKeyword \'{keyword}\' '
                f'-RegistryValue \'{value}\' '
                f'-NoRestart"'
            )
            out, err, rc = run_cmd(cmd)

            if rc == 0:
                log(f"{keyword} = {value} 설정 완료", "OK")
            else:
                log(f"{keyword} 설정 실패 또는 미지원: {err}", "WARN")

    def disable_enable(self, nic_name):
        name = ps_escape(nic_name)

        log(f"NIC '{nic_name}' Disable...")
        cmd_dis = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            f'"Disable-NetAdapter -Name \'{name}\' -Confirm:$false"'
        )
        out, err, rc = run_cmd(cmd_dis)

        if rc != 0:
            log(f"Disable 실패: {err}", "ERR")
            return False

        log("NIC Disabled", "OK")
        time.sleep(3)

        log(f"NIC '{nic_name}' Enable...")
        cmd_en = (
            'powershell -NoProfile -ExecutionPolicy Bypass -Command '
            f'"Enable-NetAdapter -Name \'{name}\' -Confirm:$false"'
        )
        out, err, rc = run_cmd(cmd_en)

        if rc != 0:
            log(f"Enable 실패: {err}", "ERR")
            return False

        log("NIC Enabled → AN 재시작됨", "OK")
        return True

    def wait_link_up(self, nic_name, timeout=20):
        log(f"링크 UP 대기 중 최대 {timeout}초...")

        for i in range(timeout):
            status, err, rc = self.get_status(nic_name)

            print(f"\r  [{i + 1:2d}s] Status: {status:<20}", end="", flush=True)

            if rc == 0 and status.lower() == "up":
                print()
                speed = self.get_link_speed(nic_name)
                log(f"링크 UP 확인: {speed}", "OK")
                return True

            time.sleep(1)

        print()
        log("링크 UP 실패", "ERR")
        return False

    def run_test(self, nic_name, repeat=3, interval=5):
        print("=" * 60)
        log(f"AN 테스트 시작: {nic_name}", "INFO")
        print("=" * 60)

        self.set_auto_negotiation(nic_name)

        for i in range(1, repeat + 1):
            print()
            log(f"===== 테스트 {i}/{repeat} =====")

            ok = self.disable_enable(nic_name)

            if ok:
                linked = self.wait_link_up(nic_name, timeout=20)
                if linked:
                    speed = self.get_link_speed(nic_name)
                    log(f"협상 결과: {speed}", "OK")
                else:
                    log("협상 실패 또는 링크 미검출", "ERR")

            if i < repeat:
                log(f"{interval}초 대기...")
                time.sleep(interval)

        print()
        print("=" * 60)
        log("테스트 완료", "OK")
        print("=" * 60)


def select_interface(nic):
    interfaces = nic.list_interfaces()

    if not interfaces:
        log("NIC를 찾을 수 없습니다.", "ERR")
        sys.exit(1)

    print()
    print("=" * 60)
    print("사용 가능한 NIC 목록")
    print("=" * 60)

    for idx, name in enumerate(interfaces):
        print(f"[{idx}] {name}")

    print()

    while True:
        choice = input("NIC 번호 선택, 또는 이름 직접 입력 Enter=0: ").strip()

        if choice == "":
            return interfaces[0]

        if choice.isdigit():
            idx = int(choice)
            if 0 <= idx < len(interfaces):
                return interfaces[idx]

        if choice in interfaces:
            return choice

        log("잘못된 선택입니다.", "WARN")


def main():
    if os.name != "nt":
        log("이 버전은 Windows 전용입니다.", "ERR")
        sys.exit(1)

    if not check_admin():
        log("관리자 권한으로 실행해야 합니다.", "ERR")
        sys.exit(1)

    nic = WindowsNIC()

    nic_name = select_interface(nic)
    log(f"선택된 NIC: {nic_name}", "OK")

    print()
    print("=" * 60)
    print("현재 NIC 상태")
    print("=" * 60)
    nic.show_adapter_info(nic_name)

    print()
    print("=" * 60)
    print("Speed / Duplex 관련 속성")
    print("=" * 60)
    nic.show_advanced_properties(nic_name)

    print()
    repeat = int(input("반복 횟수 Enter=3: ").strip() or "3")
    interval = int(input("테스트 간격 초 Enter=5: ").strip() or "5")

    nic.run_test(nic_name, repeat=repeat, interval=interval)


if __name__ == "__main__":
    main()