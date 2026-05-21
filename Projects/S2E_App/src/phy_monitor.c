/**
 * phy_monitor.c - WIZ750SR-100 PHY Monitor
 *
 * [주의사항]
 *  1. phy_monitor_init() 은 반드시 WDT_Init() 이후에 호출
 *  2. phy_link_monitor() 는 while(1) 메인루프에서만 호출
 *  3. mdio_init()은 W7500x_Board_Init()에서 이미 수행되므로 재호출 안 함
 *
 * [timerHandler 사용 함수]
 *  - getDevtime()             : devtime_sec  반환
 *  - millis()                 : devtime_msec 반환
 *  - get_phylink_downtime()   : phylink_down_time_msec 반환
 *  - set_phylink_time_check(1): 링크다운 카운터 리셋 + 타이머 시작
 *  - set_phylink_time_check(0): 링크다운 타이머 정지
 */

#include <stdio.h>
#include "W7500x_gpio.h"
#include "W7500x_miim.h"
#include "W7500x_wdt.h"
#include "timerHandler.h"
#include "phy_monitor.h"

/*──────────────────────────────────────────
 * W7500x Reset Info Register
 *──────────────────────────────────────────*/
#define RSTINFO_REG     (*(volatile uint32_t*)0x4001F010)
#define RST_POR_MSK     (0x01)
#define RST_WDT_MSK     (0x02)
#define RST_SOFT_MSK    (0x04)
#define RST_EXT_MSK     (0x08)

/*──────────────────────────────────────────
 * Init 완료 magic number
 * WDT reset 후 RAM 유지되어도 오동작 방지
 *──────────────────────────────────────────*/
#define PHY_INIT_MAGIC  0xA5

/*──────────────────────────────────────────
 * 내부 상태 변수
 *──────────────────────────────────────────*/
static PHY_SpeedMode s_mode           = PHY_MODE_AUTO;
static uint8_t       s_prev_link      = 0;
static uint8_t       s_dump_down_done = 0;  /* DOWN 1초 후 덤프 완료 여부 */
static uint8_t       s_dump_up_done   = 0;  /* UP 직후 덤프 완료 여부     */
static uint8_t       s_initialized    = 0;

/*──────────────────────────────────────────
 * PHY 레지스터 덤프
 * mdio_read 전후 WDT kick 추가
 *──────────────────────────────────────────*/
void phy_dump_registers(void)
{
    uint32_t r0, r1, r4, r5, r16, r31;

    WDT_IntClear();
    r0  = mdio_read(GPIOB, PHY_REG_CONTROL);
    r1  = mdio_read(GPIOB, PHY_REG_STATUS);
    WDT_IntClear();
    r4  = mdio_read(GPIOB, PHY_REG_AUTONEG_ADV);
    r5  = mdio_read(GPIOB, PHY_REG_LINK_PARTNER);
    WDT_IntClear();
    r16 = mdio_read(GPIOB, PHY_REG_SPECIAL_MODES);
    r31 = mdio_read(GPIOB, PHY_REG_SPECIAL_CTRL);
    WDT_IntClear();
	printf("[PHY DUMP] :  %lu sec %lu ms  DOWN:%lu ms\r\n",
           getDevtime(), millis(), get_phylink_downtime());

    printf("[PHY] Reg00(Ctrl)    =0x%04X  %s %s %s\r\n",
           (unsigned)r0,
           (r0 & PHY_CTRL_SPEED_SEL)   ? "100M"  : "10M",
           (r0 & PHY_CTRL_DUPLEX_FULL) ? "Full"  : "Half",
           (r0 & PHY_CTRL_AUTONEG_EN)  ? "AN:ON" : "AN:OFF");

    printf("[PHY] Reg01(Stat)    =0x%04X  Link:%s  AN:%s  RemFault:%s\r\n",
           (unsigned)r1,
           (r1 & PHY_STAT_LINK_UP)      ? "UP"   : "DOWN",
           (r1 & PHY_STAT_AUTONEG_DONE) ? "Done" : "No",
           (r1 & PHY_STAT_REMOTE_FAULT) ? "YES"  : "No");
	    /* ↓ AN 진행 상태 판별 추가 (r0, r1 재활용, 추가 read 없음) */
    if(r0 & PHY_CTRL_AUTONEG_EN) {
        if(!(r1 & PHY_STAT_AUTONEG_DONE) && !(r1 & PHY_STAT_LINK_UP))
            printf("[PHY] AN Status      = IN PROGRESS (협상 중)\r\n");
        else if((r1 & PHY_STAT_AUTONEG_DONE) && (r1 & PHY_STAT_LINK_UP))
            printf("[PHY] AN Status      = COMPLETE (협상 완료)\r\n");
        else if((r1 & PHY_STAT_AUTONEG_DONE) && !(r1 & PHY_STAT_LINK_UP))
            printf("[PHY] AN Status      = COMPLETE but NO LINK\r\n");
    } else {
        printf("[PHY] AN Status      = DISABLED (Fixed 모드)\r\n");
    }
    printf("[PHY] Reg04(AnAdv)   =0x%04X  Reg05(LPA)   =0x%04X\r\n",
           (unsigned)r4, (unsigned)r5);

    printf("[PHY] Reg16(SpModes) =0x%04X  Reg31(SpCtrl)=0x%04X\r\n",
           (unsigned)r16, (unsigned)r31);
printf(" ========================================================\r\n");
    WDT_IntClear();
}

/*──────────────────────────────────────────
 * PHY 속도/듀플렉스 고정 설정
 * mdio_write/read 전후 WDT kick 추가
 *──────────────────────────────────────────*/
void phy_set_fixed_mode(PHY_SpeedMode mode)
{
    uint32_t    val;
    const char *str;

    switch(mode) {
        case PHY_MODE_100F:
            val = PHY_CTRL_SPEED_SEL | PHY_CTRL_DUPLEX_FULL;
            str = "100F"; break;
        case PHY_MODE_100H:
            val = PHY_CTRL_SPEED_SEL;
            str = "100H"; break;
        case PHY_MODE_10F:
            val = PHY_CTRL_DUPLEX_FULL;
            str = "10F";  break;
        case PHY_MODE_10H:
            val = 0;
            str = "10H";  break;
        case PHY_MODE_AUTO:
        default:
            val = PHY_CTRL_AUTONEG_EN | PHY_CTRL_RESTART_AN;
            str = "Auto"; break;
    }

    WDT_IntClear();
    mdio_write(GPIOB, PHY_REG_CONTROL, val);
    WDT_IntClear();

    printf("[PHY] Mode=%s  Ctrl=0x%04X\r\n",
           str, (unsigned)mdio_read(GPIOB, PHY_REG_CONTROL));

    WDT_IntClear();
    s_mode = mode;
}

/*──────────────────────────────────────────
 * 초기화
 * 호출 위치: WDT_Init() 이후, while(1) 진입 전
 *──────────────────────────────────────────*/
void phy_monitor_init(PHY_SpeedMode mode)
{
    uint32_t rst;
    uint8_t  wdt_only;
    uint32_t i;

    /* 1. 리셋 원인 판별 및 출력 */
    rst      = RSTINFO_REG;
    wdt_only = ((rst & RST_WDT_MSK) && !(rst & RST_POR_MSK)) ? 1 : 0;

    printf("[PHY] Init  ResetCause=0x%02X", (unsigned)rst);
    if(rst & RST_POR_MSK)  printf(" POR");
    if(rst & RST_WDT_MSK)  printf(wdt_only ? " WDT!!!" : " WDT(+POR)");
    if(rst & RST_SOFT_MSK) printf(" SOFT");
    if(rst & RST_EXT_MSK)  printf(" EXT");
    printf("\r\n");

    if(wdt_only)
        printf("[PHY] *** WATCHDOG RESET DETECTED ***\r\n");

    /* 2. PHY SW Reset
     *    대기 중 WDT_IntClear() 로 WDT kick */
    WDT_IntClear();
    mdio_write(GPIOB, PHY_REG_CONTROL, PHY_CTRL_SW_RESET);
    for(i = 0; i < 500; i++) {
        WDT_IntClear();
        delay(1);
        if(!(mdio_read(GPIOB, PHY_REG_CONTROL) & PHY_CTRL_SW_RESET)) break;
    }
    WDT_IntClear();

    if(i < 500) printf("[PHY] SW Reset done (%lums)\r\n", i);
    else        printf("[PHY] SW Reset TIMEOUT!\r\n");

    /* 3. PHY 고정 모드 설정 */
    phy_set_fixed_mode(mode);

    /* 4. 링크 다운 카운터 리셋 + 타이머 시작 */
    set_phylink_time_check(1);

    /* 5. 내부 상태 초기화 */
    s_prev_link      = 0;
    s_dump_down_done = 0;
    s_dump_up_done   = 0;
    s_initialized    = PHY_INIT_MAGIC;
}

/*──────────────────────────────────────────
 * 링크 모니터링
 * while(1) 메인루프에서 호출
 * phy_monitor_init() 완료 후에만 동작
 *
 * [덤프 동작]
 *  - LINK DOWN : 1초 이상 지속 시 1회 덤프
 *  - LINK UP   : 복구 직후 1회 덤프
 *──────────────────────────────────────────*/
void phy_link_monitor(void)
{
    uint32_t r1;
    uint8_t  curr_link;

    /* magic number 가드: init 미완료 시 즉시 리턴 */
    if(s_initialized != PHY_INIT_MAGIC) return;

    WDT_IntClear();
    r1        = mdio_read(GPIOB, PHY_REG_STATUS);
    curr_link = (r1 & PHY_STAT_LINK_UP) ? 1 : 0;
    WDT_IntClear();

    /* 링크 상태 변화 감지 */
    if(curr_link != s_prev_link) {
        if(curr_link == 0) {
            /* 링크 DOWN */
            set_phylink_time_check(1);   /* 카운터 리셋 + 타이머 시작 */
            s_dump_down_done = 0;        /* DOWN 덤프 플래그 리셋     */
            s_dump_up_done   = 0;        /* UP 덤프 플래그 리셋       */
            printf("[PHY] LINK DOWN %lu sec %lu ms\r\n",
                   getDevtime(), millis());
        } else {
            /* 링크 UP */
            printf("[PHY] LINK UP  %lu sec %lu ms (DOWN=%lu ms)\r\n",
                   getDevtime(), millis(), get_phylink_downtime());
            set_phylink_time_check(0);   /* 타이머 정지               */
            s_dump_down_done = 0;        /* DOWN 덤프 플래그 리셋     */
            s_dump_up_done   = 0;        /* UP 덤프 1회 허용          */

            /* 고정 모드 재적용 (PHY 내부 리셋 후 Control 레지스터 복구) */
            if(s_mode != PHY_MODE_AUTO)
                phy_set_fixed_mode(s_mode);
        }
        s_prev_link = curr_link;
    }

    /* LINK DOWN: 1초 이상 지속 → 덤프 (1회만) */
    if(!curr_link && !s_dump_down_done &&
       get_phylink_downtime() >= PHY_LINK_DOWN_THRESHOLD_MS) {
        printf("[PHY] ############# LINK DOWN DUMP #############\r\n");
        WDT_IntClear();
        phy_dump_registers();
        s_dump_down_done = 1;
    }

    /* LINK UP: 복구 직후 → 덤프 (1회만) */
    if(curr_link && !s_dump_up_done) {
        printf("[PHY] ############# LINK UP DUMP #############\r\n");
        WDT_IntClear();
        phy_dump_registers();
        s_dump_up_done = 1;
    }
}
