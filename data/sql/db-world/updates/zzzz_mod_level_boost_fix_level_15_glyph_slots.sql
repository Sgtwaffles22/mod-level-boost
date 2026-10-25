-- The bundled level-15 minor glyphs were seeded into slot index 3, which is locked at level 15.
-- Move only the known bundled glyph entries to the unlocked minor slot; leave other templates alone.
UPDATE `mod_boost_glyphs`
SET `glyph_slot` = 1
WHERE `glyph_slot` = 3
  AND ((`player_class` = 'Warrior' AND `glyph_id` = 485)
    OR (`player_class` = 'Paladin' AND `glyph_id` = 457)
    OR (`player_class` = 'Hunter' AND `glyph_id` = 441)
    OR (`player_class` = 'Rogue' AND `glyph_id` = 464)
    OR (`player_class` = 'Priest' AND `glyph_id` = 458)
    OR (`player_class` = 'Shaman' AND `glyph_id` = 470)
    OR (`player_class` = 'Mage' AND `glyph_id` = 451)
    OR (`player_class` = 'Warlock' AND `glyph_id` = 482)
    OR (`player_class` = 'Druid' AND `glyph_id` = 433));
