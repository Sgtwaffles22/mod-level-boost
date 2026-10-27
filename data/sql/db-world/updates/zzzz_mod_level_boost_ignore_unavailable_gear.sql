-- Remove deprecated or explicitly unavailable items from existing boost gear templates.
-- The runtime ignore check deliberately leaves these equipment slots empty rather than substituting gear.
DELETE gear
FROM `mod_boost_gear` AS gear
LEFT JOIN `item_template` AS item ON item.`entry` = gear.`item_entry`
WHERE (item.`Flags` & 16) <> 0
   OR LOWER(item.`name`) LIKE '%test%'
   OR gear.`item_entry` IN (905, 906, 907, 908, 909, 2588, 3529, 4667, 4670);
