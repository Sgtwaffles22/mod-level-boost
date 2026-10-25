# Level Boost

This AzerothCore module offers a configurable boost from a race's starting quest giver. The default target is level 15 and the default grant is 100 gold.

## Configuration

Copy `conf/mod_level_boost.conf.dist` to `conf/mod_level_boost.conf` and edit:

```ini
LevelBoost.Enable = 1
LevelBoost.OneTimeOnly = 1
LevelBoost.Level = 15
LevelBoost.Gold = 100
Bags.Enable = 1
Bags.Item = 4240
Bags.Count = 4
ClassQuests.Enable = 1
```

`LevelBoost.Level` accepts levels from 1 through the server's `MaxPlayerLevel`; the module clamps the configured value to that range. `LevelBoost.Gold` is in whole gold: 1g is 10,000 copper, so `100` grants 100g (1,000,000 copper) and `0` grants none. The grant respects the core's money cap. ClassQuests.Enable defaults to 1. Boost previews show the target level, actual gold grant, class quest count, reward entry count, and quests left incomplete. Fixed item rewards are granted; a choice reward is selected only when its first usable option matches the character's class, race, faction, and target level. If no choice item is usable, that quest stays incomplete. Quests with unsupported non-item rewards, such as spells, reputation, honor, titles, bonus talents, arena points, or reward mail, are also left incomplete and logged so they can still be completed normally. Quest XP and money are not granted. Reward items are mailed if the full reward does not fit in the character's inventory.

`Bags.Item` defaults to item 4240 (Woolen Bag), an 8-slot bag, and `Bags.Count` defaults to four. The module equips bags in the character's four bag slots (19–22), filling empty slots first; bags that cannot be equipped are sent by mail. This gives a level-15 character up to 32 extra inventory slots without using higher-level bags.

## Starter templates

The world database SQL creates generic level-15 templates for all 27 class/spec combinations. Apply the module's base world SQL to install them and the starting NPC text. Apply `data/sql/db-characters/base/level_boost_claim.sql` to install the one-time claim table when `LevelBoost.OneTimeOnly = 1` (the default).

- Gear is selected from the core's `item_template` table, limited to class- and race-compatible items with a required level of 15 or lower.
- Talents spend six points in the selected tree's first tier. Any points left at a higher configured boost level remain available.
- One class-appropriate minor glyph is equipped in the level-15 glyph slot.

The gear and talent starter data is aimed at level 15. For a different target, boost a character to that level, configure its gear, talents, and glyphs, then run `.levelboost extract <spec>` as an administrator to replace that class/spec's templates. For example: `.levelboost extract Retribution`.

## Boost flow

After choosing a specialization, the gossip menu shows a preview with a confirm button. Each character can claim the boost once by default. Set LevelBoost.OneTimeOnly to 0 to allow repeat claims.

## Commands

- `.levelboost reload` reloads the gear, talent, glyph, spell, and proficiency templates.
- `.levelboost validate` checks that all 27 specs have templates and reports invalid or level-incompatible data.
- `.levelboost extract <spec>` saves the current character's gear, talents, and all six glyph slots for that specialization.
