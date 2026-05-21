# WIZ750SR

WIZnet Serial to Ethernet(S2E) module based on W7500 chip, WIZ107/108SR S2E compatible device

<p align="center">
  <img width="40%" src="https://docs.wiznet.io/img/products/wiz750sr/wiz750sr_rev1.0_main_1024x693.png" />
</p>

<p align="center">
  <img width="40%" src="https://docs.wiznet.io/img/products/s2e_module/wiz750sr-1xx/wiz750sr-100.png" />
</p>

<p align="center">
  <img width="40%" src="https://docs.wiznet.io/img/products/wiz750sr-110/wiz750sr-110_main.png" />
</p>

For more details, please refer to [WIZ750SR document page](https://docs.wiznet.io/Product/S2E-Module/WIZ750SR/) in [WIZnet Documents](https://docs.wiznet.io/).

---

## Project Overview

This repository manages **WIZ750SR firmware projects starting from v1.4.5**, with a focus on various debug mode variants and experimental features.

Each project is maintained in its **own branch**, while the **main branch README is always kept up to date** as the central reference for all projects.

---

## Branch Strategy

| Branch | Description |
|--------|-------------|
| `main` | README, documentation hub, and Python utilities (always up to date) |
| `phy-auto-nego-control` | PHY Auto-Negotiation control via MDIO |

> **Note:** All project branches are derived from **v1.4.5** as the base firmware version.  
> The `main` branch README is the single source of truth and is updated whenever a new project or change is introduced.

---

## Projects

### 1. PHY Auto-Negotiation Control (`phy-auto-nego-control` branch)

The current WIZ750SR firmware uses PHY Auto-Negotiation by default. This project overrides that behavior by directly controlling PHY register values through the **MDIO interface**, allowing fixed-speed and duplex configurations.

**Purpose:**
- Disable Auto-Nego and manually set PHY link speed/duplex via MDIO register writes
- Useful for environments where Auto-Nego causes compatibility or timing issues
- Provides a debug/test foundation for PHY-level diagnostics

**Key implementation points:**
- MDIO read/write routines targeting the embedded PHY (IC Plus IP101G)
- Register-level control of speed (10/100M) and duplex (Half/Full)
- Debug output to verify PHY register state at runtime

---

### 2. Python Utilities (`main` branch — `python/` folder)

A collection of Python scripts managed directly in the main branch, designed to work alongside the firmware projects.

**Purpose:**
- Serial/Ethernet communication test tools
- Integration scripts that interact with running firmware projects
- Automation helpers for configuration, testing, and data capture across multiple projects

---

## Base Firmware Version

All projects in this repository start from **v1.4.5 Stable** of the WIZ750SR firmware.

---

## Tools & References

- [ISP Tool](https://docs.wiznet.io/Product/iMCU/W7500/documents/appnote/how-to-use-isp-tool)
- [Configuration Tool (GUI)](https://github.com/Wiznet/WIZnet-S2E-Tool-GUI)
- [Configuration Tool (CLI)](https://github.com/Wiznet/WIZnet-S2E-Tool)
- [WIZ750SR Command Manual](https://docs.wiznet.io/Product/S2E-Module/WIZ750SR/command-manual-EN)
- [WIZ750SR Configuration Tool Manual](https://docs.wiznet.io/Product/S2E-Module/WIZ750SR/configuration-tool-manual-new-EN)
- [WIZVSP](https://docs.wiznet.io/Product/S2E-Module/WIZ750SR/download#wiz-vsp)
