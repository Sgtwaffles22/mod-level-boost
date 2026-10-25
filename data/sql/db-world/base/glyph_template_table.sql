DROP TABLE IF EXISTS `mod_boost_glyphs`;
CREATE TABLE IF NOT EXISTS `mod_boost_glyphs` (
    `player_class` VARCHAR(50) NOT NULL,
    `player_spec` VARCHAR(50) NOT NULL,
    `glyph_slot` TINYINT UNSIGNED NOT NULL DEFAULT '0',
    `glyph_id` SMALLINT UNSIGNED NOT NULL DEFAULT '0'
) COLLATE='utf8mb4_general_ci' COMMENT='Glyph Templates';

-- glyph_id stores the GlyphProperties ID expected by Player::SetGlyph.
-- One class-appropriate minor glyph is assigned to unlocked slot 3 for every level-15 specialization.
INSERT INTO `mod_boost_glyphs` (`player_class`, `player_spec`, `glyph_slot`, `glyph_id`) VALUES
    ('Warrior', 'Arms', 3, 485),
    ('Warrior', 'Fury', 3, 485),
    ('Warrior', 'Protection', 3, 485),
    ('Paladin', 'Holy', 3, 457),
    ('Paladin', 'Protection', 3, 457),
    ('Paladin', 'Retribution', 3, 457),
    ('Hunter', 'Beast Mastery', 3, 441),
    ('Hunter', 'Marksmanship', 3, 441),
    ('Hunter', 'Survival', 3, 441),
    ('Rogue', 'Assassination', 3, 464),
    ('Rogue', 'Combat', 3, 464),
    ('Rogue', 'Subtlety', 3, 464),
    ('Priest', 'Discipline', 3, 458),
    ('Priest', 'Holy', 3, 458),
    ('Priest', 'Shadow', 3, 458),
    ('Shaman', 'Elemental', 3, 470),
    ('Shaman', 'Enhancement', 3, 470),
    ('Shaman', 'Restoration', 3, 470),
    ('Mage', 'Arcane', 3, 451),
    ('Mage', 'Fire', 3, 451),
    ('Mage', 'Frost', 3, 451),
    ('Warlock', 'Affliction', 3, 482),
    ('Warlock', 'Demonology', 3, 482),
    ('Warlock', 'Destruction', 3, 482),
    ('Druid', 'Balance', 3, 433),
    ('Druid', 'Feral', 3, 433),
    ('Druid', 'Restoration', 3, 433);
