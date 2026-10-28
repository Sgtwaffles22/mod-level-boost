-- Level-15 best-in-slot candidates ranked from obtainable equipment of all qualities.
-- Candidates are ranked by specialization stat priorities and weapon damage.
-- Random property IDs select an obtainable stat variant for each item.
-- Existing extracted green templates are migrated and all BIS overrides are preserved.
CREATE TABLE IF NOT EXISTS `mod_boost_bis_gear` (
    `player_class` VARCHAR(50) NOT NULL,
    `player_spec` VARCHAR(50) NOT NULL,
    `player_race` VARCHAR(50) NOT NULL,
    `equipment_slot` INT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `random_property` INT NOT NULL DEFAULT 0,
    PRIMARY KEY (`player_class`, `player_spec`, `player_race`, `equipment_slot`)
) COLLATE='utf8mb4_general_ci' COMMENT='Level-15 ranked gear templates';

CREATE TABLE IF NOT EXISTS `mod_boost_bis_overrides` (
    `player_class` VARCHAR(50) NOT NULL,
    `player_spec` VARCHAR(50) NOT NULL,
    `player_race` VARCHAR(50) NOT NULL,
    PRIMARY KEY (`player_class`, `player_spec`, `player_race`)
) COLLATE='utf8mb4_general_ci' COMMENT='Extracted ranked gear templates';

-- Preserve customizations from the previous green template table.
INSERT IGNORE INTO `mod_boost_bis_gear`
SELECT g.* FROM `mod_boost_green_gear` AS g
JOIN `mod_boost_green_overrides` AS old_overrides
  ON old_overrides.`player_class` = g.`player_class`
 AND old_overrides.`player_spec` = g.`player_spec`
 AND old_overrides.`player_race` = g.`player_race`
LEFT JOIN `mod_boost_bis_overrides` AS overrides
  ON overrides.`player_class` = g.`player_class`
 AND overrides.`player_spec` = g.`player_spec`
 AND overrides.`player_race` = g.`player_race`
WHERE overrides.`player_class` IS NULL;
INSERT IGNORE INTO `mod_boost_bis_overrides` SELECT * FROM `mod_boost_green_overrides`;

INSERT IGNORE INTO `mod_boost_bis_gear`
    (`player_class`, `player_spec`, `player_race`, `equipment_slot`, `item_entry`, `random_property`)
SELECT preset.`player_class`, preset.`player_spec`, races.`player_race`, preset.`equipment_slot`, preset.`item_entry`, preset.`random_property`
FROM (
    SELECT 'Warrior' AS `player_class`, 'Arms' AS `player_spec`, 2 AS `equipment_slot`, 10657 AS `item_entry`, 0 AS `random_property` -- Talbar Mantle
    UNION ALL SELECT 'Warrior', 'Arms', 4, 9783, 155 -- Raider's Chestpiece
    UNION ALL SELECT 'Warrior', 'Arms', 5, 5425, 0 -- Runescale Girdle
    UNION ALL SELECT 'Warrior', 'Arms', 6, 15312, 1553 -- Feral Leggings
    UNION ALL SELECT 'Warrior', 'Arms', 7, 9776, 136 -- Bandit Boots
    UNION ALL SELECT 'Warrior', 'Arms', 8, 15112, 1550 -- Rigid Bracelets
    UNION ALL SELECT 'Warrior', 'Arms', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Warrior', 'Arms', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Warrior', 'Arms', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Warrior', 'Arms', 14, 6575, 97 -- Defender Cloak
    UNION ALL SELECT 'Warrior', 'Arms', 15, 5423, 0 -- Boahn's Fang
    UNION ALL SELECT 'Warrior', 'Arms', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Warrior', 'Fury', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Warrior', 'Fury', 4, 9783, 155 -- Raider's Chestpiece
    UNION ALL SELECT 'Warrior', 'Fury', 5, 5425, 0 -- Runescale Girdle
    UNION ALL SELECT 'Warrior', 'Fury', 6, 15312, 1553 -- Feral Leggings
    UNION ALL SELECT 'Warrior', 'Fury', 7, 9776, 136 -- Bandit Boots
    UNION ALL SELECT 'Warrior', 'Fury', 8, 15112, 1550 -- Rigid Bracelets
    UNION ALL SELECT 'Warrior', 'Fury', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Warrior', 'Fury', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Warrior', 'Fury', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Warrior', 'Fury', 14, 6575, 97 -- Defender Cloak
    UNION ALL SELECT 'Warrior', 'Fury', 15, 5423, 0 -- Boahn's Fang
    UNION ALL SELECT 'Warrior', 'Fury', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Warrior', 'Protection', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Warrior', 'Protection', 4, 9783, 154 -- Raider's Chestpiece
    UNION ALL SELECT 'Warrior', 'Protection', 5, 5425, 0 -- Runescale Girdle
    UNION ALL SELECT 'Warrior', 'Protection', 6, 15493, 1189 -- Bloodspattered Loincloth
    UNION ALL SELECT 'Warrior', 'Protection', 7, 14129, 1187 -- Ritual Sandals
    UNION ALL SELECT 'Warrior', 'Protection', 8, 15507, 96 -- Grunt's Bracers
    UNION ALL SELECT 'Warrior', 'Protection', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Warrior', 'Protection', 10, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Warrior', 'Protection', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Warrior', 'Protection', 14, 6575, 96 -- Defender Cloak
    UNION ALL SELECT 'Warrior', 'Protection', 15, 12976, 0 -- Ironpatch Blade
    UNION ALL SELECT 'Warrior', 'Protection', 16, 9778, 96 -- Bandit Buckler
    UNION ALL SELECT 'Warrior', 'Protection', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Paladin', 'Holy', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Paladin', 'Holy', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Paladin', 'Holy', 5, 15110, 762 -- Rigid Belt
    UNION ALL SELECT 'Paladin', 'Holy', 6, 15312, 2033 -- Feral Leggings
    UNION ALL SELECT 'Paladin', 'Holy', 7, 15498, 2033 -- Outrunner's Slippers
    UNION ALL SELECT 'Paladin', 'Holy', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Paladin', 'Holy', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Paladin', 'Holy', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Paladin', 'Holy', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Paladin', 'Holy', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Paladin', 'Holy', 15, 15222, 2030 -- Barbed Club
    UNION ALL SELECT 'Paladin', 'Holy', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Paladin', 'Protection', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Paladin', 'Protection', 4, 9783, 154 -- Raider's Chestpiece
    UNION ALL SELECT 'Paladin', 'Protection', 5, 5425, 0 -- Runescale Girdle
    UNION ALL SELECT 'Paladin', 'Protection', 6, 15493, 1189 -- Bloodspattered Loincloth
    UNION ALL SELECT 'Paladin', 'Protection', 7, 14129, 1187 -- Ritual Sandals
    UNION ALL SELECT 'Paladin', 'Protection', 8, 15507, 96 -- Grunt's Bracers
    UNION ALL SELECT 'Paladin', 'Protection', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Paladin', 'Protection', 10, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Paladin', 'Protection', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Paladin', 'Protection', 14, 6575, 96 -- Defender Cloak
    UNION ALL SELECT 'Paladin', 'Protection', 15, 12976, 0 -- Ironpatch Blade
    UNION ALL SELECT 'Paladin', 'Protection', 16, 9778, 96 -- Bandit Buckler
    UNION ALL SELECT 'Paladin', 'Retribution', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Paladin', 'Retribution', 4, 9783, 155 -- Raider's Chestpiece
    UNION ALL SELECT 'Paladin', 'Retribution', 5, 5425, 0 -- Runescale Girdle
    UNION ALL SELECT 'Paladin', 'Retribution', 6, 15312, 1553 -- Feral Leggings
    UNION ALL SELECT 'Paladin', 'Retribution', 7, 9776, 136 -- Bandit Boots
    UNION ALL SELECT 'Paladin', 'Retribution', 8, 15112, 1550 -- Rigid Bracelets
    UNION ALL SELECT 'Paladin', 'Retribution', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Paladin', 'Retribution', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Paladin', 'Retribution', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Paladin', 'Retribution', 14, 6575, 97 -- Defender Cloak
    UNION ALL SELECT 'Paladin', 'Retribution', 15, 5423, 0 -- Boahn's Fang
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 4, 15018, 1710 -- Lupine Vest
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 5, 14173, 235 -- Buccaneer's Cord
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 7, 4788, 0 -- Agile Boots
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 8, 6563, 1707 -- Shimmering Bracers
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 10, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 14, 6585, 93 -- Scouting Cloak
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 15, 15248, 151 -- Gleaming Claymore
    UNION ALL SELECT 'Hunter', 'Beast Mastery', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Hunter', 'Marksmanship', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Hunter', 'Marksmanship', 4, 15018, 1710 -- Lupine Vest
    UNION ALL SELECT 'Hunter', 'Marksmanship', 5, 14173, 235 -- Buccaneer's Cord
    UNION ALL SELECT 'Hunter', 'Marksmanship', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Hunter', 'Marksmanship', 7, 4788, 0 -- Agile Boots
    UNION ALL SELECT 'Hunter', 'Marksmanship', 8, 6563, 1707 -- Shimmering Bracers
    UNION ALL SELECT 'Hunter', 'Marksmanship', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Hunter', 'Marksmanship', 10, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Hunter', 'Marksmanship', 14, 6585, 93 -- Scouting Cloak
    UNION ALL SELECT 'Hunter', 'Marksmanship', 15, 15248, 151 -- Gleaming Claymore
    UNION ALL SELECT 'Hunter', 'Marksmanship', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Hunter', 'Survival', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Hunter', 'Survival', 4, 15018, 1710 -- Lupine Vest
    UNION ALL SELECT 'Hunter', 'Survival', 5, 14173, 235 -- Buccaneer's Cord
    UNION ALL SELECT 'Hunter', 'Survival', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Hunter', 'Survival', 7, 4788, 0 -- Agile Boots
    UNION ALL SELECT 'Hunter', 'Survival', 8, 6563, 1707 -- Shimmering Bracers
    UNION ALL SELECT 'Hunter', 'Survival', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Hunter', 'Survival', 10, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Hunter', 'Survival', 14, 6585, 93 -- Scouting Cloak
    UNION ALL SELECT 'Hunter', 'Survival', 15, 15248, 151 -- Gleaming Claymore
    UNION ALL SELECT 'Hunter', 'Survival', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Rogue', 'Assassination', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Rogue', 'Assassination', 4, 15018, 151 -- Lupine Vest
    UNION ALL SELECT 'Rogue', 'Assassination', 5, 15110, 675 -- Rigid Belt
    UNION ALL SELECT 'Rogue', 'Assassination', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Rogue', 'Assassination', 7, 9776, 678 -- Bandit Boots
    UNION ALL SELECT 'Rogue', 'Assassination', 8, 15112, 93 -- Rigid Bracelets
    UNION ALL SELECT 'Rogue', 'Assassination', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Rogue', 'Assassination', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Rogue', 'Assassination', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Rogue', 'Assassination', 14, 6585, 93 -- Scouting Cloak
    UNION ALL SELECT 'Rogue', 'Assassination', 15, 12976, 0 -- Ironpatch Blade
    UNION ALL SELECT 'Rogue', 'Assassination', 16, 935, 0 -- Night Watch Shortsword
    UNION ALL SELECT 'Rogue', 'Assassination', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Rogue', 'Combat', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Rogue', 'Combat', 4, 15018, 151 -- Lupine Vest
    UNION ALL SELECT 'Rogue', 'Combat', 5, 15110, 675 -- Rigid Belt
    UNION ALL SELECT 'Rogue', 'Combat', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Rogue', 'Combat', 7, 9776, 678 -- Bandit Boots
    UNION ALL SELECT 'Rogue', 'Combat', 8, 15112, 93 -- Rigid Bracelets
    UNION ALL SELECT 'Rogue', 'Combat', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Rogue', 'Combat', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Rogue', 'Combat', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Rogue', 'Combat', 14, 6585, 93 -- Scouting Cloak
    UNION ALL SELECT 'Rogue', 'Combat', 15, 12976, 0 -- Ironpatch Blade
    UNION ALL SELECT 'Rogue', 'Combat', 16, 935, 0 -- Night Watch Shortsword
    UNION ALL SELECT 'Rogue', 'Combat', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Rogue', 'Subtlety', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Rogue', 'Subtlety', 4, 15018, 151 -- Lupine Vest
    UNION ALL SELECT 'Rogue', 'Subtlety', 5, 15110, 675 -- Rigid Belt
    UNION ALL SELECT 'Rogue', 'Subtlety', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Rogue', 'Subtlety', 7, 9776, 678 -- Bandit Boots
    UNION ALL SELECT 'Rogue', 'Subtlety', 8, 15112, 93 -- Rigid Bracelets
    UNION ALL SELECT 'Rogue', 'Subtlety', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Rogue', 'Subtlety', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Rogue', 'Subtlety', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Rogue', 'Subtlety', 14, 6585, 93 -- Scouting Cloak
    UNION ALL SELECT 'Rogue', 'Subtlety', 15, 12976, 0 -- Ironpatch Blade
    UNION ALL SELECT 'Rogue', 'Subtlety', 16, 935, 0 -- Night Watch Shortsword
    UNION ALL SELECT 'Rogue', 'Subtlety', 17, 3040, 0 -- Hunter's Muzzle Loader
    UNION ALL SELECT 'Priest', 'Discipline', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Priest', 'Discipline', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Priest', 'Discipline', 5, 14173, 760 -- Buccaneer's Cord
    UNION ALL SELECT 'Priest', 'Discipline', 6, 6540, 760 -- Willow Pants
    UNION ALL SELECT 'Priest', 'Discipline', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Priest', 'Discipline', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Priest', 'Discipline', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Priest', 'Discipline', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Priest', 'Discipline', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Priest', 'Discipline', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Priest', 'Discipline', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Priest', 'Discipline', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Priest', 'Discipline', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Priest', 'Holy', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Priest', 'Holy', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Priest', 'Holy', 5, 14173, 760 -- Buccaneer's Cord
    UNION ALL SELECT 'Priest', 'Holy', 6, 6540, 760 -- Willow Pants
    UNION ALL SELECT 'Priest', 'Holy', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Priest', 'Holy', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Priest', 'Holy', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Priest', 'Holy', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Priest', 'Holy', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Priest', 'Holy', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Priest', 'Holy', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Priest', 'Holy', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Priest', 'Holy', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Priest', 'Shadow', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Priest', 'Shadow', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Priest', 'Shadow', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Priest', 'Shadow', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Priest', 'Shadow', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Priest', 'Shadow', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Priest', 'Shadow', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Priest', 'Shadow', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Priest', 'Shadow', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Priest', 'Shadow', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Priest', 'Shadow', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Priest', 'Shadow', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Priest', 'Shadow', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Shaman', 'Elemental', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Shaman', 'Elemental', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Shaman', 'Elemental', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Shaman', 'Elemental', 6, 15312, 2033 -- Feral Leggings
    UNION ALL SELECT 'Shaman', 'Elemental', 7, 9776, 846 -- Bandit Boots
    UNION ALL SELECT 'Shaman', 'Elemental', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Shaman', 'Elemental', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Shaman', 'Elemental', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Shaman', 'Elemental', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Shaman', 'Elemental', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Shaman', 'Elemental', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Shaman', 'Elemental', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Shaman', 'Enhancement', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Shaman', 'Enhancement', 4, 15018, 1553 -- Lupine Vest
    UNION ALL SELECT 'Shaman', 'Enhancement', 5, 15110, 136 -- Rigid Belt
    UNION ALL SELECT 'Shaman', 'Enhancement', 6, 15312, 1553 -- Feral Leggings
    UNION ALL SELECT 'Shaman', 'Enhancement', 7, 9776, 678 -- Bandit Boots
    UNION ALL SELECT 'Shaman', 'Enhancement', 8, 15112, 1550 -- Rigid Bracelets
    UNION ALL SELECT 'Shaman', 'Enhancement', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Shaman', 'Enhancement', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Shaman', 'Enhancement', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Shaman', 'Enhancement', 14, 9779, 674 -- Bandit Cloak
    UNION ALL SELECT 'Shaman', 'Enhancement', 15, 4575, 1553 -- Medicine Staff
    UNION ALL SELECT 'Shaman', 'Restoration', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Shaman', 'Restoration', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Shaman', 'Restoration', 5, 15110, 762 -- Rigid Belt
    UNION ALL SELECT 'Shaman', 'Restoration', 6, 15312, 2033 -- Feral Leggings
    UNION ALL SELECT 'Shaman', 'Restoration', 7, 9776, 760 -- Bandit Boots
    UNION ALL SELECT 'Shaman', 'Restoration', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Shaman', 'Restoration', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Shaman', 'Restoration', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Shaman', 'Restoration', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Shaman', 'Restoration', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Shaman', 'Restoration', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Shaman', 'Restoration', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Mage', 'Arcane', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Mage', 'Arcane', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Mage', 'Arcane', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Mage', 'Arcane', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Mage', 'Arcane', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Mage', 'Arcane', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Mage', 'Arcane', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Mage', 'Arcane', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Mage', 'Arcane', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Mage', 'Arcane', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Mage', 'Arcane', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Mage', 'Arcane', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Mage', 'Arcane', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Mage', 'Fire', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Mage', 'Fire', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Mage', 'Fire', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Mage', 'Fire', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Mage', 'Fire', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Mage', 'Fire', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Mage', 'Fire', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Mage', 'Fire', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Mage', 'Fire', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Mage', 'Fire', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Mage', 'Fire', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Mage', 'Fire', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Mage', 'Fire', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Mage', 'Frost', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Mage', 'Frost', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Mage', 'Frost', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Mage', 'Frost', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Mage', 'Frost', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Mage', 'Frost', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Mage', 'Frost', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Mage', 'Frost', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Mage', 'Frost', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Mage', 'Frost', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Mage', 'Frost', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Mage', 'Frost', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Mage', 'Frost', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Warlock', 'Affliction', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Warlock', 'Affliction', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Warlock', 'Affliction', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Warlock', 'Affliction', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Warlock', 'Affliction', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Warlock', 'Affliction', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Warlock', 'Affliction', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Warlock', 'Affliction', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Warlock', 'Affliction', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Warlock', 'Affliction', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Warlock', 'Affliction', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Warlock', 'Affliction', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Warlock', 'Affliction', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Warlock', 'Demonology', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Warlock', 'Demonology', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Warlock', 'Demonology', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Warlock', 'Demonology', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Warlock', 'Demonology', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Warlock', 'Demonology', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Warlock', 'Demonology', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Warlock', 'Demonology', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Warlock', 'Demonology', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Warlock', 'Demonology', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Warlock', 'Demonology', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Warlock', 'Demonology', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Warlock', 'Demonology', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Warlock', 'Destruction', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Warlock', 'Destruction', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Warlock', 'Destruction', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Warlock', 'Destruction', 6, 6540, 2030 -- Willow Pants
    UNION ALL SELECT 'Warlock', 'Destruction', 7, 14129, 760 -- Ritual Sandals
    UNION ALL SELECT 'Warlock', 'Destruction', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Warlock', 'Destruction', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Warlock', 'Destruction', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Warlock', 'Destruction', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Warlock', 'Destruction', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Warlock', 'Destruction', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Warlock', 'Destruction', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Warlock', 'Destruction', 17, 31270, -83 -- Banshee Rod
    UNION ALL SELECT 'Druid', 'Balance', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Druid', 'Balance', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Druid', 'Balance', 5, 4786, 0 -- Wise Man's Belt
    UNION ALL SELECT 'Druid', 'Balance', 6, 15312, 2033 -- Feral Leggings
    UNION ALL SELECT 'Druid', 'Balance', 7, 9776, 846 -- Bandit Boots
    UNION ALL SELECT 'Druid', 'Balance', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Druid', 'Balance', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Druid', 'Balance', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Druid', 'Balance', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Druid', 'Balance', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Druid', 'Balance', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Druid', 'Balance', 16, 15971, 2030 -- Aboriginal Rod
    UNION ALL SELECT 'Druid', 'Feral', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Druid', 'Feral', 4, 15018, 151 -- Lupine Vest
    UNION ALL SELECT 'Druid', 'Feral', 5, 15110, 675 -- Rigid Belt
    UNION ALL SELECT 'Druid', 'Feral', 6, 15312, 151 -- Feral Leggings
    UNION ALL SELECT 'Druid', 'Feral', 7, 9776, 678 -- Bandit Boots
    UNION ALL SELECT 'Druid', 'Feral', 8, 15112, 93 -- Rigid Bracelets
    UNION ALL SELECT 'Druid', 'Feral', 9, 10413, 0 -- Gloves of the Fang
    UNION ALL SELECT 'Druid', 'Feral', 10, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Druid', 'Feral', 11, 11981, 842 -- Lead Band
    UNION ALL SELECT 'Druid', 'Feral', 14, 9779, 674 -- Bandit Cloak
    UNION ALL SELECT 'Druid', 'Feral', 15, 4575, 1553 -- Medicine Staff
    UNION ALL SELECT 'Druid', 'Restoration', 2, 10657, 0 -- Talbar Mantle
    UNION ALL SELECT 'Druid', 'Restoration', 4, 14120, 152 -- Aboriginal Robe
    UNION ALL SELECT 'Druid', 'Restoration', 5, 15110, 762 -- Rigid Belt
    UNION ALL SELECT 'Druid', 'Restoration', 6, 15312, 2033 -- Feral Leggings
    UNION ALL SELECT 'Druid', 'Restoration', 7, 9776, 760 -- Bandit Boots
    UNION ALL SELECT 'Druid', 'Restoration', 8, 6563, 2030 -- Shimmering Bracers
    UNION ALL SELECT 'Druid', 'Restoration', 9, 12977, 0 -- Magefist Gloves
    UNION ALL SELECT 'Druid', 'Restoration', 10, 11981, 94 -- Lead Band
    UNION ALL SELECT 'Druid', 'Restoration', 11, 11965, 1098 -- Quartz Ring
    UNION ALL SELECT 'Druid', 'Restoration', 14, 6575, 2030 -- Defender Cloak
    UNION ALL SELECT 'Druid', 'Restoration', 15, 3184, 2030 -- Hook Dagger
    UNION ALL SELECT 'Druid', 'Restoration', 16, 15971, 2030 -- Aboriginal Rod
) AS preset
JOIN (
    SELECT 'Warrior' AS `player_class`, 'Human' AS `player_race`
    UNION ALL SELECT 'Warrior', 'Orc'
    UNION ALL SELECT 'Warrior', 'Dwarf'
    UNION ALL SELECT 'Warrior', 'Nightelf'
    UNION ALL SELECT 'Warrior', 'Undead'
    UNION ALL SELECT 'Warrior', 'Tauren'
    UNION ALL SELECT 'Warrior', 'Gnome'
    UNION ALL SELECT 'Warrior', 'Troll'
    UNION ALL SELECT 'Warrior', 'Bloodelf'
    UNION ALL SELECT 'Warrior', 'Draenei'
    UNION ALL SELECT 'Paladin', 'Human'
    UNION ALL SELECT 'Paladin', 'Dwarf'
    UNION ALL SELECT 'Paladin', 'Bloodelf'
    UNION ALL SELECT 'Paladin', 'Draenei'
    UNION ALL SELECT 'Hunter', 'Orc'
    UNION ALL SELECT 'Hunter', 'Dwarf'
    UNION ALL SELECT 'Hunter', 'Nightelf'
    UNION ALL SELECT 'Hunter', 'Tauren'
    UNION ALL SELECT 'Hunter', 'Troll'
    UNION ALL SELECT 'Hunter', 'Bloodelf'
    UNION ALL SELECT 'Hunter', 'Draenei'
    UNION ALL SELECT 'Rogue', 'Human'
    UNION ALL SELECT 'Rogue', 'Orc'
    UNION ALL SELECT 'Rogue', 'Dwarf'
    UNION ALL SELECT 'Rogue', 'Nightelf'
    UNION ALL SELECT 'Rogue', 'Undead'
    UNION ALL SELECT 'Rogue', 'Gnome'
    UNION ALL SELECT 'Rogue', 'Troll'
    UNION ALL SELECT 'Rogue', 'Bloodelf'
    UNION ALL SELECT 'Priest', 'Human'
    UNION ALL SELECT 'Priest', 'Dwarf'
    UNION ALL SELECT 'Priest', 'Nightelf'
    UNION ALL SELECT 'Priest', 'Undead'
    UNION ALL SELECT 'Priest', 'Troll'
    UNION ALL SELECT 'Priest', 'Bloodelf'
    UNION ALL SELECT 'Priest', 'Draenei'
    UNION ALL SELECT 'Shaman', 'Orc'
    UNION ALL SELECT 'Shaman', 'Tauren'
    UNION ALL SELECT 'Shaman', 'Troll'
    UNION ALL SELECT 'Shaman', 'Draenei'
    UNION ALL SELECT 'Mage', 'Human'
    UNION ALL SELECT 'Mage', 'Undead'
    UNION ALL SELECT 'Mage', 'Gnome'
    UNION ALL SELECT 'Mage', 'Troll'
    UNION ALL SELECT 'Mage', 'Bloodelf'
    UNION ALL SELECT 'Mage', 'Draenei'
    UNION ALL SELECT 'Warlock', 'Human'
    UNION ALL SELECT 'Warlock', 'Orc'
    UNION ALL SELECT 'Warlock', 'Undead'
    UNION ALL SELECT 'Warlock', 'Gnome'
    UNION ALL SELECT 'Warlock', 'Bloodelf'
    UNION ALL SELECT 'Druid', 'Nightelf'
    UNION ALL SELECT 'Druid', 'Tauren'
) AS races ON races.`player_class` = preset.`player_class`
LEFT JOIN `mod_boost_bis_overrides` AS overrides ON overrides.`player_class` = preset.`player_class` AND overrides.`player_spec` = preset.`player_spec` AND overrides.`player_race` = races.`player_race`
WHERE overrides.`player_class` IS NULL;
