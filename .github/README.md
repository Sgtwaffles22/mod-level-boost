# Level Boost

## Description
"Level Boost" is a module for AzerothCore that offers an optional level boost to level 15, available as a gossip option at each race's starting zone initial quest giver. This module is designed to enhance the player experience by providing a quick start option for players who wish to skip the early leveling phase.

## Features
- Automatic boost to any specified level via a gossip option with the initial quest giver in each race's starting zone.
- Destruction of starting items and equipping players with appropriate gear, talents, and glyphs based on configuration.
- Configurable options for enabling or disabling specific features like learning spells, proficiencies, talents, dual spec, and destroying gear.
- Ability to reload tables and extract gear templates based on player's current gear.

## Configuration
The module includes several configurable options in the `mod_level_15_boost.conf` file:


```ini
# Level 15 Boost
LevelBoost.Enable = 1

# Learn Spells
LearnSpells.Enable = 1

# Learn Proficiencies
LearnProficiencies.Enable = 1

# Learn Talents
LearnTalents.Enable = 1

# Learn Glyphs
LearnGlyphs.Enable = 1

# Enable Dual Spec
DualSpec.Enable = 1

# Destroy Starting Gear
DestroyGear.Enable = 1
```

## Usage Instructions

### Renaming .conf.dist to .conf
The mod_level_15_boost.conf.dist file is a template file that you will need to rename to mod_level_15_boost.conf before using the module. To do this, simply open the file in a text editor and change the name from .conf.dist to .conf.

### Commands

- `.levelboost reload`: Reload the module's tables.
- `.levelboost extract [spec]`: Extract and update the gear template to match what the player running the command is currently wearing. Example: `.levelboost extract Retribution`

## Support and Contributions

For support, questions, or contributions, please use the GitHub Issues page associated with this project.

