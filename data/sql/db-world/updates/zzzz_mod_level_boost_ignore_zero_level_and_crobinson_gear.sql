-- Remove explicitly unavailable items, zero-item-level gear, and CRobinson gear from existing boost templates.
DELETE gear
FROM `mod_boost_gear` AS gear
LEFT JOIN `item_template` AS item ON item.`entry` = gear.`item_entry`
WHERE gear.`item_entry` IN (4728, 40225)
   OR item.`ItemLevel` = 0
   OR LOWER(item.`name`) LIKE 'crobinson %';
