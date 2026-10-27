# Level-15 green gear selection

The default gear templates use uncommon (green) items for all supported classes and specializations. The choices are ranked for leveling by their relevant stats and weapon damage. The templates include a specific random property or suffix when an item has random stats; for example, Hunters can receive an Agility or ranged attack power variant instead of a random caster variant.

The ranking is a practical estimate, not a combat simulation or a complete best-in-slot list for every playstyle. It does not score item spells, proc effects, set bonuses, enchantments added by players, or interactions across the full set. Where appropriate, it compares a two-handed weapon with a one-handed weapon and off-hand item. Its stat priorities and relative weights are:

| Profile | Specializations | Stat weights |
| --- | --- | --- |
| Melee | Arms, Fury, Retribution | Strength 4, Agility 2, Stamina 1.5, attack power 2, critical rating 1.5 |
| Tank | Warrior and Paladin Protection | Stamina 4, Strength 3, defense rating 3, dodge and parry rating 2, Agility and attack power 1.5, block rating 1 |
| Rogue | Assassination, Combat, Subtlety | Agility 4, Strength 2, attack power 2, Stamina 1.5, critical rating 1.5 |
| Hunter | Beast Mastery, Marksmanship, Survival | Agility 4, attack power and ranged attack power 2, Stamina 1.5, Intellect 1, critical rating 1.5 |
| Enhancement | Enhancement Shaman | Agility and Strength 3, attack power 2, Stamina 1.5, Intellect 1 |
| Feral | Feral Druid | Agility 4, Strength 3, Stamina and attack power 2 |
| Caster | Elemental, Balance, Shadow, Mage and Warlock specs | Intellect and spell power 4, Spirit 2, Stamina 1.5, critical rating 1 |
| Healer | Holy Paladin, Discipline and Holy Priest, Restoration Shaman and Druid | Intellect and spell power 4, Spirit 3, Stamina 1.5 |

Weapon damage per second is weighted at 4 for melee and tank main-hand or off-hand weapons, 5 for Hunter ranged weapons, and 0.2 for other weapons. Tanks also receive 0.025 points per armor and 0.05 points per shield block value. Ties favor higher item level, then lower item ID and random-property ID.

The starter pool was built from level-15-eligible, item-level 1–22 green equipment in a representative AzerothCore world database. It includes unrestricted items found in spawned-creature loot, reachable reference loot, or spawned vendors, and excludes profession requirements, deprecated and known unavailable items. One exception to the item-level cap is [Talbar Mantle](https://www.wowhead.com/wotlk/item=10657/talbar-mantle), an item-level 26 quest reward available to both factions at level 15. The pool does not include every valid source, such as other quest rewards, crafted items, or container-only loot. Consequently, an empty slot means no suitable item was selected from this pool; it does not prove that no green item exists for that slot.

The templates live in `mod_boost_green_gear` and are loaded by default with `Gear.Random.Enable = 0`. The database updater creates this table automatically. Existing `mod_boost_gear` templates are retained as Random Gear; set `Gear.Random.Enable = 1` and restart or reload the server config to use them. `.levelboost extract <spec>` saves gear into the currently selected table. In green mode it records only allowed green equipment and its random property or suffix. Run `.levelboost reload` after extracting. Extracted green templates are marked in `mod_boost_green_overrides` so reapplying the update preserves their empty slots and item choices.

These templates are designed for level 15 and do not change automatically when `LevelBoost.Level` changes. A template item is skipped if the server's item or random-property data differs, the character cannot equip it, or the item is excluded by the module's ignore rules. Quest rewards and class items are controlled by their separate configuration options.
