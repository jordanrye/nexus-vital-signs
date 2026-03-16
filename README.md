[![](https://discordapp.com/api/guilds/410828272679518241/widget.png?style=banner2)](https://discord.gg/Mvk7W7gjE4)

![](https://img.shields.io/github/downloads/jordanrye/nexus-vital-signs/total?color=7a85d7)
![](https://img.shields.io/github/v/release/jordanrye/nexus-vital-signs?color=7a85d7)

# Vital Signs

**Vital Signs** is a customisable party and squad frame addon for Guild Wars 2, inspired by [Cell](https://github.com/enderneko/Cell) and [Reffect](https://github.com/Zerthox/gw2-reffect). It allows players to monitor the health, effects and status of their group through user-defined layouts.

![Screenshot taken using Live Preview data.](./resources/screenshot.png)

**WARNING:** Vital Signs performs memory reading. Please review our [Addon Policy](#addon-policy) to learn how we handle game data and endeavour to operate within ArenaNet's guidelines.

## Features

### Layout Customisation

- **Templates:** Choose between two distinct styles when creating a layout:
    - **Grid Layouts:** Traditional layout where unit frames are arranged in rows and columns. 
    - **Radial Layouts:** Circular layout where unit frames are arranged in slices around a central point.
- **Indicators:** Compose frames using a variety of visual elements:
    - **Icons:** Track boons, conditions, and other effects with custom icons.
    - **Text Labels:** Display names, health percentages, and custom labels with adjustable fonts and styles.
    - **Colours, Borders & Highlights:** Add visual emphasis to frames based on triggers.
    - **Resource Bars:** Visualise health, barrier, and shroud (Necromancer/Specter) resources.
- **Colour Palettes:** 
    - **Generic:** Based on the Guild Wars 2 colour scheme of the party and squad UI.
    - **Profession:** Quickly identify teammates with health bars using profession-specific colours.
- **Live Preview:** Visualise your changes with dummy data as you edit.

### Dynamic Triggers

Every element in a layout is controlled by a trigger system:

- **Health States:** React to the health, barrier and status of players.
- **Effects:** Track a curated set of Boons, Conditions, Auras, and other effects. Triggers can be created for **active status**, **stack count**, or **duration threshold**.
- **Professions:** Add indicators based on a character's Profession or Elite Specialisation.

### Context Switching

Automatically switch between different layouts based on your group type:

- **Party** (5 players)
- **Raid** (10 players)
- **Squad** (50 players)

### Quick Actions

- **Click-to-Select:** Click a unit frame to target players in-game, allowing for quick support or inspection.

## Installation & Configuration

1. Ensure [Nexus](https://raidcore.gg/Nexus) is installed.
2. Download [`VitalSigns.dll`](https://github.com/jordanrye/nexus-vital-signs/releases/latest) and place it into your `<Guild Wars 2>/addons` directory.
3. Launch the game and open the Nexus addons window (default `Ctrl+O`) to configure Vital Signs:
    - **General:** Assign specific layouts to be used in different group settings.
    - **Layout Editor:** Create, delete, and modify the visual components of your layouts.
    - **Presets:** Manage global colour palettes and text styles for a unified look.

## Addon Policy

Vital Signs utilises memory reading to access internal data from the Guild Wars 2 game client. To prevent potential abuse or the development of unauthorised cheats, the memory-access portion of the codebase remains closed-source and is restricted to a small group of verified developers.

This addon is designed to operate within the guidelines of the Guild Wars 2 [Third-Party Programs](https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs) and [Macros & Macro Use](https://help.guildwars2.com/hc/en-us/articles/360013762153-Policy-Macros-and-Macro-Use) policies. In areas of ambiguity, development is guided by community consensus and available statements from ArenaNet staff.

If you have any questions or concerns regarding this Addon Policy, we encourage you to reach out for a discussion via the `#vital-signs` channel in the [Nexus Discord](https://discord.gg/Mvk7W7gjE4).

### Compliance

To preserve game integrity and adhere to the policy outlined above, limitations have been set for Data Parity and Mode Restrictions.

#### Data Parity

Vital Signs will only display information which is natively presented in the game client. The following table contains a non-exhaustive list of examples as to what may and may not be permitted under this rule:

| Data | Availability | Default Party UI | Simple Party UI | Squad Grid UI | Target Frame |
| --- | --- | :---: | :---: | :---: | :---: |
| Health & Barrier | Health and barrier can be displayed at all times. A percentage value can be given for health, but must be limited to zero decimal places. Raw numerical values for the current and maximum health of other players is **not** permitted. | ✔ | ✔ | ✔ | ✔ |
| Life Force | Life Force can be displayed while Shroud is active. A percentage value can be given for Life Force, but must be limited to zero decimal places. | ✔ | ✔ | ✔ | ✔ |
| Health (while in Shroud) | Health and Life Force can be displayed simultaneously while in Shroud. | ✔ | ✔ | | ✔ |
| Boons & Conditions | Duration and stacks are permitted. | ✔ | ✔ | | ✔ |
| Auras | Limited to the party context (i.e., not available while in a squad). Duration is permitted. | ✔ | | | ✔ |
| Misc. (Superspeed, Stealth, and Revealed) | Limited to the party context. Duration is permitted. | ✔ | | | ✔ |
| Control Effects | Limited to the party context. Indicated effects including Stun and Daze are permitted, while other effects such as as Knockback, Launch and Pull are **not** permitted. | ✔ | | | ✔ |
| Squad Border Effects | Raid mechanics which are indicated via a border in the Squad Grid UI are permitted (e.g., pink border for Exile's Embrace). Duration is **not** permitted. | | | ✔ | |
| Profession & Elite Specialisation | Current Profession and Elite Specialisation (if applicable) are permitted. Viewing individual trait selections (i.e., gear check) is **not** permitted. | ✔ | ✔ | ✔ | ✔ |
| Levels | Character Level, Mastery Level and Agony Resistance (in Fractals) are permitted. | ✔ | ✔ | | ✔ |

#### Mode Restrictions

The addon is automatically disabled in competitive zones (i.e., PvP and WvW) to preserve game integrity and prevent the user from gaining an unfair advantage over other players.

### Disclaimer

While every effort is made to ensure that Vital Signs complies with ArenaNet's current policies, the use of any third-party software is at the sole discretion of the player. The developers of this addon are not responsible for any actions taken against your account. ArenaNet's policies are subject to change without notice; we encourage you to stay informed and decide for yourself whether you are comfortable with the risks of using third-party programs.
