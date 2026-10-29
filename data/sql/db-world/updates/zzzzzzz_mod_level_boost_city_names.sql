-- City-specific questions, including an update for servers with the original prompt.
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `Probability0`)
VALUES
(70001, 'Would you like to travel to Stormwind?', 'Would you like to travel to Stormwind?', 0, 1),
(70002, 'Would you like to travel to Ironforge?', 'Would you like to travel to Ironforge?', 0, 1),
(70003, 'Would you like to travel to Darnassus?', 'Would you like to travel to Darnassus?', 0, 1),
(70004, 'Would you like to travel to The Exodar?', 'Would you like to travel to The Exodar?', 0, 1),
(70005, 'Would you like to travel to Orgrimmar?', 'Would you like to travel to Orgrimmar?', 0, 1),
(70006, 'Would you like to travel to Thunder Bluff?', 'Would you like to travel to Thunder Bluff?', 0, 1),
(70007, 'Would you like to travel to Undercity?', 'Would you like to travel to Undercity?', 0, 1),
(70008, 'Would you like to travel to Silvermoon City?', 'Would you like to travel to Silvermoon City?', 0, 1)
ON DUPLICATE KEY UPDATE `text0_0` = VALUES(`text0_0`), `text0_1` = VALUES(`text0_1`),
    `BroadcastTextID0` = 0, `Probability0` = 1;
