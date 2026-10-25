DROP TABLE IF EXISTS `mod_boost_gear`;
CREATE TABLE IF NOT EXISTS `mod_boost_gear` (
    `player_class` VARCHAR(50) NOT NULL,
    `player_spec` VARCHAR(50) NOT NULL,
    `player_race` VARCHAR(50) NOT NULL,
    `equipment_slot` INT(10) UNSIGNED NOT NULL DEFAULT '0',
    `item_entry` INT(10) UNSIGNED NOT NULL DEFAULT '0'
) COLLATE='utf8mb4_general_ci' COMMENT='Gear Templates';

-- Build generic level-15 gear templates from this core's item_template table.
-- The item filters keep each selection wearable by the class and race in the row.
INSERT INTO `mod_boost_gear`
    (`player_class`, `player_spec`, `player_race`, `equipment_slot`, `item_entry`)
SELECT
    seed.`player_class`,
    seed.`player_spec`,
    seed.`player_race`,
    seed.`equipment_slot`,
    seed.`item_entry`
FROM (
    WITH races (`race_id`, `player_race`, `race_mask`) AS (
        SELECT 1, 'Human', 1 UNION ALL
        SELECT 2, 'Orc', 2 UNION ALL
        SELECT 3, 'Dwarf', 4 UNION ALL
        SELECT 4, 'Nightelf', 8 UNION ALL
        SELECT 5, 'Undead', 16 UNION ALL
        SELECT 6, 'Tauren', 32 UNION ALL
        SELECT 7, 'Gnome', 64 UNION ALL
        SELECT 8, 'Troll', 128 UNION ALL
        SELECT 10, 'Bloodelf', 512 UNION ALL
        SELECT 11, 'Draenei', 1024
    ),
    classes (`player_class`, `class_id`, `armor_subclass`, `weapon_subclasses`, `ranged_subclasses`, `can_shield`, `race_ids`) AS (
        SELECT 'Warrior', 1, 3, '0,1,4,5,6,7,8,10,13,15', '2,3,18', 1, '1,2,3,4,5,6,7,8,10,11'
        UNION ALL
        SELECT 'Paladin', 2, 3, '0,1,4,5,6,7,8', '', 1, '1,3,10,11'
        UNION ALL
        SELECT 'Hunter', 3, 2, '0,1,6,7,8,10,13,15', '2,3,18', 0, '2,3,4,6,8,10,11'
        UNION ALL
        SELECT 'Rogue', 4, 2, '4,7,13,15', '2,3,18', 0, '1,2,3,4,5,8,10'
        UNION ALL
        SELECT 'Priest', 5, 1, '4,10,15', '', 0, '1,3,4,5,8,10,11'
        UNION ALL
        SELECT 'Shaman', 7, 2, '0,1,4,5,10,13,15', '', 1, '2,6,8,11'
        UNION ALL
        SELECT 'Mage', 8, 1, '7,10,15', '', 0, '1,5,7,8,10,11'
        UNION ALL
        SELECT 'Warlock', 9, 1, '7,10,15', '', 0, '1,2,5,7,10'
        UNION ALL
        SELECT 'Druid', 11, 2, '4,5,6,10,13,15', '', 0, '4,6'
    ),
    class_races AS (
        SELECT c.*, r.`player_race`, r.`race_mask`
        FROM classes c
        JOIN races r ON FIND_IN_SET(r.`race_id`, c.`race_ids`) > 0
    ),
    specs (`player_class`, `player_spec`) AS (
        SELECT 'Warrior', 'Arms'
        UNION ALL
        SELECT 'Warrior', 'Fury'
        UNION ALL
        SELECT 'Warrior', 'Protection'
        UNION ALL
        SELECT 'Paladin', 'Holy'
        UNION ALL
        SELECT 'Paladin', 'Protection'
        UNION ALL
        SELECT 'Paladin', 'Retribution'
        UNION ALL
        SELECT 'Hunter', 'Beast Mastery'
        UNION ALL
        SELECT 'Hunter', 'Marksmanship'
        UNION ALL
        SELECT 'Hunter', 'Survival'
        UNION ALL
        SELECT 'Rogue', 'Assassination'
        UNION ALL
        SELECT 'Rogue', 'Combat'
        UNION ALL
        SELECT 'Rogue', 'Subtlety'
        UNION ALL
        SELECT 'Priest', 'Discipline'
        UNION ALL
        SELECT 'Priest', 'Holy'
        UNION ALL
        SELECT 'Priest', 'Shadow'
        UNION ALL
        SELECT 'Shaman', 'Elemental'
        UNION ALL
        SELECT 'Shaman', 'Enhancement'
        UNION ALL
        SELECT 'Shaman', 'Restoration'
        UNION ALL
        SELECT 'Mage', 'Arcane'
        UNION ALL
        SELECT 'Mage', 'Fire'
        UNION ALL
        SELECT 'Mage', 'Frost'
        UNION ALL
        SELECT 'Warlock', 'Affliction'
        UNION ALL
        SELECT 'Warlock', 'Demonology'
        UNION ALL
        SELECT 'Warlock', 'Destruction'
        UNION ALL
        SELECT 'Druid', 'Balance'
        UNION ALL
        SELECT 'Druid', 'Feral'
        UNION ALL
        SELECT 'Druid', 'Restoration'
    ),
    slot_types (`equipment_slot`, `inventory_type`, `item_kind`) AS (
        SELECT 0, 1, 'armor' UNION ALL
        SELECT 1, 2, 'armor' UNION ALL
        SELECT 2, 3, 'armor' UNION ALL
        SELECT 4, 5, 'armor' UNION ALL
        SELECT 4, 20, 'armor' UNION ALL
        SELECT 5, 6, 'armor' UNION ALL
        SELECT 6, 7, 'armor' UNION ALL
        SELECT 7, 8, 'armor' UNION ALL
        SELECT 8, 9, 'armor' UNION ALL
        SELECT 9, 10, 'armor' UNION ALL
        SELECT 10, 11, 'armor' UNION ALL
        SELECT 14, 16, 'armor' UNION ALL
        SELECT 15, 13, 'weapon' UNION ALL
        SELECT 15, 17, 'weapon' UNION ALL
        SELECT 15, 21, 'weapon' UNION ALL
        SELECT 16, 14, 'shield' UNION ALL
        SELECT 17, 15, 'ranged' UNION ALL
        SELECT 17, 19, 'wand'
    ),
    candidates AS (
        SELECT
            cr.`player_class`,
            sp.`player_spec`,
            cr.`player_race`,
            st.`equipment_slot`,
            it.`entry` AS `item_entry`,
            ROW_NUMBER() OVER (
                PARTITION BY cr.`player_class`, sp.`player_spec`, cr.`player_race`, st.`equipment_slot`
                ORDER BY
                    CASE
                        WHEN cr.`player_class` = 'Paladin' AND sp.`player_spec` = 'Retribution'
                             AND st.`equipment_slot` = 15 AND it.`InventoryType` = 17 THEN 0
                        WHEN cr.`player_class` = 'Paladin' AND sp.`player_spec` = 'Retribution'
                             AND st.`equipment_slot` = 15 THEN 1
                        ELSE 0
                    END,
                    it.`Quality` DESC, it.`ItemLevel` DESC, it.`entry`
            ) AS `row_num`
        FROM class_races cr
        JOIN specs sp ON sp.`player_class` = cr.`player_class`
        CROSS JOIN slot_types st
        CROSS JOIN `item_template` it
        WHERE it.`RequiredLevel` <= 15
          AND it.`ItemLevel` <= 22
          AND it.`Quality` BETWEEN 1 AND 2
          AND it.`RequiredSkill` = 0
          AND it.`RequiredSkillRank` = 0
          AND it.`InventoryType` = st.`inventory_type`
          AND (it.`InventoryType` <> 17 OR (cr.`player_class` = 'Paladin' AND sp.`player_spec` = 'Retribution'))
          AND (it.`AllowableClass` = -1 OR (it.`AllowableClass` & (1 << (cr.`class_id` - 1))) <> 0)
          AND (it.`AllowableRace` = -1 OR (it.`AllowableRace` & cr.`race_mask`) <> 0)
          AND (
                (st.`item_kind` = 'armor' AND it.`class` = 4 AND it.`subclass` = cr.`armor_subclass`)
             OR (st.`item_kind` = 'weapon' AND it.`class` = 2 AND FIND_IN_SET(it.`subclass`, cr.`weapon_subclasses`) > 0)
             OR (st.`item_kind` = 'shield' AND cr.`can_shield` = 1
                 AND NOT (cr.`player_class` = 'Paladin' AND sp.`player_spec` = 'Retribution')
                 AND it.`class` = 4 AND it.`subclass` = 6)
             OR (st.`item_kind` = 'ranged' AND it.`class` = 2 AND FIND_IN_SET(it.`subclass`, cr.`ranged_subclasses`) > 0)
             OR (st.`item_kind` = 'wand' AND cr.`player_class` IN ('Priest', 'Mage', 'Warlock') AND it.`class` = 2 AND it.`subclass` = 19)
          )
    )
    SELECT `player_class`, `player_spec`, `player_race`, `equipment_slot`, `item_entry`, `row_num`
    FROM candidates
) AS seed
WHERE seed.`row_num` = 1;
