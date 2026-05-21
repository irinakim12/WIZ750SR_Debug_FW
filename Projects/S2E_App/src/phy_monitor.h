#ifndef __PHY_MONITOR_H__
#define __PHY_MONITOR_H__

#include <stdint.h>

/* PHY Control Register (Reg 0x00) 비트 정의 */
#define PHY_CTRL_SPEED_SEL      (0x01 << 13)  // Speed: 1=100M, 0=10M
#define PHY_CTRL_AUTONEG_EN     (0x01 << 12)  // Auto-Negotiation Enable
#define PHY_CTRL_POWER_DOWN     (0x01 << 11)  // Power Down
#define PHY_CTRL_RESTART_AN     (0x01 << 9)   // Restart Auto-Negotiation
#define PHY_CTRL_DUPLEX_FULL    (0x01 << 8)   // Duplex: 1=Full, 0=Half
#define PHY_CTRL_SW_RESET       (0x01 << 15)  // Software Reset (self-clear)

/* PHY Status Register (Reg 0x01) 비트 정의 */
#define PHY_STAT_AUTONEG_DONE   (0x01 << 5)   // Auto-Neg Complete
#define PHY_STAT_REMOTE_FAULT   (0x01 << 4)   // Remote Fault
#define PHY_STAT_LINK_UP        (0x01 << 2)   // Link Status: 1=UP

/* IP101G 레지스터 주소 */
#define PHY_REG_CONTROL         0x00
#define PHY_REG_STATUS          0x01
#define PHY_REG_PHYSID1         0x02
#define PHY_REG_PHYSID2         0x03
#define PHY_REG_AUTONEG_ADV     0x04
#define PHY_REG_LINK_PARTNER    0x05
#define PHY_REG_AUTONEG_EXP     0x06
#define PHY_REG_SPECIAL_MODES   0x16          // IP101G 전용: 링크속도 상태
#define PHY_REG_SPECIAL_CTRL    0x1F          // IP101G 전용: RMII/MII 설정

/* PHY 속도/듀플렉스 모드 선택 */
typedef enum {
    PHY_MODE_AUTO = 0,   // Auto-Negotiation
    PHY_MODE_100F = 1,   // 100Mbps Full Duplex
    PHY_MODE_100H = 2,   // 100Mbps Half Duplex
    PHY_MODE_10F  = 3,   // 10Mbps  Full Duplex
    PHY_MODE_10H  = 4,   // 10Mbps  Half Duplex
} PHY_SpeedMode;

/* 링크 다운 덤프 임계값 (ms) */
#define PHY_LINK_DOWN_THRESHOLD_MS  1000

/* 함수 선언 */
void phy_monitor_init(PHY_SpeedMode mode);
void phy_set_fixed_mode(PHY_SpeedMode mode);
void phy_link_monitor(void);
void phy_dump_registers(void);

#endif /* __PHY_MONITOR_H__ */