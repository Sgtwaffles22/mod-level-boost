# <img src="icon.png" alt="" width="64" height="64" style="vertical-align:middle"> Level Boost

Level Boost is an AzerothCore module that lets a level-1 character choose a specialization and boost at a supported starting-area NPC. The default target is level 15, and the default grant is 100 gold.

## Fork and attribution

This repository is a fork of [fischerlol/mod-level-15-boost](https://github.com/fischerlol/mod-level-15-boost), originally created by [fischerlol](https://github.com/fischerlol). This fork is maintained as [Sgtwaffles22/mod-level-boost](https://github.com/Sgtwaffles22/mod-level-boost). It retains the original GPL-3.0 license; see [LICENSE](LICENSE).

## Installation

Place this module in your AzerothCore checkout's `modules` directory and build AzerothCore with the module enabled. When the server starts with AzerothCore's database updater enabled, it automatically discovers and applies this module's SQL files. No manual SQL import is needed. The gear seed reads from the core's `item_template` table, so the core world database must already be installed.

- SQL in `data/sql/db-world/` creates and seeds the spell, proficiency, gear, talent, and glyph templates; configures the supported starting NPCs; and registers the `.levelboost reload` command.
- The world updates also create `mod_boost_green_gear`, the default level-15 green gear templates. The original templates remain available as Random Gear.
- SQL in `data/sql/db-characters/` creates the claim table used when `LevelBoost.OneTimeOnly = 1`.

The database updater tracks applied SQL. Several template seed files drop and recreate their tables, so editing an already-applied seed file can cause custom or extracted template data to be replaced when the updater reapplies it.

## Configuration

Copy `conf/mod_level_boost.conf.dist` from the module into your server's config directory as `mod_level_boost.conf`, then change the settings you want. These are the distributed defaults:

```ini
LevelBoost.Enable = 1
LevelBoost.OneTimeOnly = 1
LevelBoost.Level = 15
LevelBoost.Gold = 100
Bags.Enable = 1
Bags.Item = 4240
Bags.Count = 4
ClassQuests.Enable = 1
ClassQuests.GrantItems = 1
ClassItems.Enable = 1
Gear.Random.Enable = 0
LearnSpells.Enable = 1
LearnProficiencies.Enable = 1
LearnTalents.Enable = 1
LearnGlyphs.Enable = 1
DualSpec.Enable = 0
DestroyGear.Enable = 1
```

- `LevelBoost.Level` is the target level. The module clamps it to the range 1 through the server's `MaxPlayerLevel`.
- `LevelBoost.Gold` is a whole-gold amount: 1 gold is 10,000 copper, so the default `100` grants 1,000,000 copper. Set it to `0` to grant no gold. The core's money cap still applies.
- `Bags.Item` is an item-template ID. The default, item 4240 (Woolen Bag), has 8 slots. `Bags.Count` is limited to the four bag slots. The module fills empty slots and does not mail bags that it cannot equip.
- `ClassQuests.Enable` controls whether eligible, non-repeatable class quests through the target level are marked rewarded. Quest completion also learns applicable quest reward spells; it does not grant quest XP or money.
- `ClassQuests.GrantItems` independently controls whether valid fixed rewards and the first usable choice item from those quests are granted. It defaults to `1` and only takes effect when `ClassQuests.Enable = 1`. The module grants only the missing quantity when the character partly owns an item, avoids duplicate item entries during one boost, and mails rewards if the inventory cannot hold them.
- `ClassItems.Enable` controls additional class-specific items granted after the boost (currently Shaman totems). It defaults to `1`.
- `Gear.Random.Enable` defaults to `0` and uses the ranked green gear templates. Set it to `1`, then restart or reload the server config to use the original Random Gear templates in `mod_boost_gear`, including existing custom templates. Despite the name, the original seed ranks eligible items by quality and item level; it does not randomly roll each slot.
- `LearnSpells.Enable` teaches eligible class spells from `mod_boost_class_spells`. `LearnProficiencies.Enable` teaches eligible class proficiencies from `mod_boost_class_proficiencies`. The corresponding world SQL files must be applied for these features to have data.
- `LearnTalents.Enable` applies the selected specialization's talent template. `LearnGlyphs.Enable` applies its glyph template when the character has unlocked the slot.
- `DualSpec.Enable` defaults to `0` (disabled). `DestroyGear.Enable` defaults to `1`; when enabled, the boost removes the character's equipped gear before applying the new gear. If disabled, existing equipped items are kept, so occupied slots may keep their current item.

With `LevelBoost.OneTimeOnly = 1`, each character can claim the boost once. If class-quest completion or spell learning is enabled, the NPC offers an update option to recheck eligible quests and missing class spells for a character that already claimed the boost. Set the option to `0` to allow repeat claims.

## Starter templates

The SQL seed provides gear, talent, and glyph templates for all 27 class/spec combinations; gear templates are also race-specific. The defaults are designed for level 15:

- The default green templates rank obtainable green items by each specialization's relevant stats and weapon damage. They include the chosen random-stat variant, use equipment available at level 15, and exclude profession-gated gear, trinkets, and known unavailable items. An empty slot stays empty if the template has no suitable item or the server cannot equip it. Arms and Fury Warriors and Retribution Paladins use two-handed weapons; Protection Warriors use a shield, and Rogues use distinct one-handed weapons. See [green gear selection](docs/green-gear.md) for the priorities and source limitations.
- Talent templates spend six points in the selected tree's first tier. At higher boost levels, unspent points remain available.
- The seed assigns one class-appropriate minor glyph to the level-15 glyph slot for each specialization.

Changing `LevelBoost.Level` does not regenerate the starter templates. To customize them, set up a character at the level you want to support, equip the desired gear, choose its talents and glyphs, then run `.levelboost extract <spec>` as an in-game administrator, followed by `.levelboost reload`. For example: `.levelboost extract Retribution`. Gear is saved for that class, specialization, and race in the currently selected gear table; talent and glyph templates are saved for that class and specialization. In green mode, extraction saves green equipment that passes the ignore list, including its random-stat variant. Other equip checks still apply during the boost.

## Using the boost

Talk to a supported starting-area NPC with a level-1 character, choose the boost option, then select a specialization. The boost is applied immediately.

## Commands

- `.levelboost reload` reloads gear, talent, glyph, spell, and proficiency templates from the world database. Use it after changing those database tables; it does not reload the config file.
- `.levelboost validate` reports missing templates and invalid or incompatible template data. It can be run in-game by an administrator or from the server console.
- `.levelboost extract <spec>` saves the current character's gear, talents, and glyphs for the specified specialization. Run it in-game as an administrator on the character whose build you want to save.
