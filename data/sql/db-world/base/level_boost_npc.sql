set @base_text = 70000;

-- add creature script to each starting creature
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 823;   -- Deputy Willem (Human)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 2079;  -- Conservator Ilthalaine (Night Elf)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 16475; -- Megelon (Draenei)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 658;   -- Sten Stoutarm (Dwarf/Gnome)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 15278; -- Magistrix Erona (Blood Elf)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 1568;  -- Undertaker Mordo (Undead)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 10176; -- Kaltunk (Orc/Troll)
UPDATE `creature_template` SET `ScriptName` = 'BoostNPC_Template' WHERE `entry` = 2980;  -- Grull Hawkwind (Tauren)

set @confirm_text = 70001;
DELETE FROM npc_text WHERE ID IN (@base_text, @confirm_text);
INSERT INTO npc_text (ID, text0_0) VALUES
(@base_text, 'Choose your specialization:'),
(@confirm_text, 'Review the boost preview and confirm to continue.');
