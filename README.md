# Level Boost

Level Boost is an AzerothCore module that lets a level-1 character choose a specialization and boost at a supported starting-area NPC. The default target is level 15, and the default grant is 100 gold.

## Installation

Place this module in your AzerothCore checkout's `modules` directory and build AzerothCore. After importing the core world database, apply the SQL files in `data/sql/db-world/base/` to your world database, then start or restart the server. The gear seed reads from the core's `item_template` table.

- `class_spells_table.sql` and `class_proficiencies_table.sql` create the spell and proficiency data used by the boost.
- `gear_template_table.sql`, `talent_template_table.sql`, and `glyph_template_table.sql` create and seed the starter builds.
- `level_boost_npc.sql` attaches the module script and dialogue to the supported starting NPCs.
- `command.sql` registers the `.levelboost reload` command.

If `LevelBoost.OneTimeOnly = 1`, also apply `data/sql/db-characters/base/level_boost_claim.sql` to your characters database.

The gear, talent, glyph, spell, and proficiency seed scripts drop and recreate their template tables. Running them again replaces any custom or extracted data in those tables.

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
- `ClassQuests.Enable` marks eligible, non-repeatable class quests through the target level as rewarded. It grants valid fixed rewards and the first usable choice item, grants only the missing quantity when the character partly owns an item, and avoids granting the same item entry twice during one boost. Learnable quest spells are granted when their conditions match. Quest XP and money are not granted; item rewards are mailed if the inventory cannot hold them.
- `LearnSpells.Enable` teaches eligible class spells from `mod_boost_class_spells`. `LearnProficiencies.Enable` teaches eligible class proficiencies from `mod_boost_class_proficiencies`. The corresponding world SQL files must be applied for these features to have data.
- `LearnTalents.Enable` applies the selected specialization's talent template. `LearnGlyphs.Enable` applies its glyph template when the character has unlocked the slot.
- `DualSpec.Enable` defaults to `0` (disabled). `DestroyGear.Enable` defaults to `1`; when enabled, the boost removes the character's equipped gear before applying the new gear. If disabled, existing equipped items are kept, so occupied slots may keep their current item.

With `LevelBoost.OneTimeOnly = 1`, each character can claim the boost once. If class-quest completion or spell learning is enabled, the NPC offers an update option to recheck eligible quests and missing class spells for a character that already claimed the boost. Set the option to `0` to allow repeat claims.

## Starter templates

The SQL seed provides gear, talent, and glyph templates for all 27 class/spec combinations; gear templates are also race-specific. The defaults are designed for level 15:

- The gear seed selects items usable by the character's class and race, with required level 15 or lower, item level 22 or lower, common or uncommon quality, and no profession-skill requirement. Trinkets are excluded. Eligible off-hand items are applied when available; Rogues receive a distinct one-handed weapon. For Retribution Paladins, an eligible two-handed weapon takes priority over off-hand gear. If a custom template contains profession-gated gear, the boost looks for a compatible replacement and skips the item if none is found.
- Talent templates spend six points in the selected tree's first tier. At higher boost levels, unspent points remain available.
- The seed assigns one class-appropriate minor glyph to the level-15 glyph slot for each specialization.

Changing `LevelBoost.Level` does not regenerate the starter templates. To customize them, set up a character at the level you want to support, equip the desired gear, choose its talents and glyphs, then run `.levelboost extract <spec>` as an in-game administrator. For example: `.levelboost extract Retribution`. Gear is saved for that class, specialization, and race; the talent and glyph templates are saved for that class and specialization.

## Using the boost

Talk to a supported starting-area NPC with a level-1 character, choose the boost option, then select a specialization. The boost is applied immediately.

## Commands

- `.levelboost reload` reloads gear, talent, glyph, spell, and proficiency templates from the world database. Use it after changing those database tables; it does not reload the config file.
- `.levelboost validate` reports missing templates and invalid or incompatible template data. It can be run in-game by an administrator or from the server console.
- `.levelboost extract <spec>` saves the current character's gear, talents, and glyphs for the specified specialization. Run it in-game as an administrator on the character whose build you want to save.
