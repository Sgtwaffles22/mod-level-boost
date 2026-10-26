#include "mod_level_boost.h"
#include "mod_level_boost_internal.h"
#include "DBCStores.h"
#include <limits>
#include <array>

namespace
{
std::array<uint32, 4> constexpr IGNORED_GEAR_ITEM_ENTRIES = {
    2588, // Deprecated Red Leather Mask
    3529, // Black Night Elf Helm
    4667, // Deprecated Battle Chain Pauldrons
    4670  // Deprecated Ancestral Mantle
};

bool IsIgnoredGearItem(uint32 itemEntry)
{
    for (uint32 ignoredItemEntry : IGNORED_GEAR_ITEM_ENTRIES)
        if (itemEntry == ignoredItemEntry)
            return true;

    return false;
}

ItemTemplate const* FindGearAlternative(Player* player, ItemTemplate const* original)
{
    if (!original)
        return nullptr;

    auto const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    if (!itemTemplates)
        return nullptr;

    ItemTemplate const* bestAlternative = nullptr;
    for (auto const& [itemId, candidate] : *itemTemplates)
    {
        if (IsIgnoredGearItem(itemId) || candidate.HasFlag(ITEM_FLAG_DEPRECATED) || candidate.Class != original->Class || candidate.SubClass != original->SubClass ||
            candidate.InventoryType != original->InventoryType || LevelBoostInternal::HasProfessionSkillRequirement(&candidate) ||
            candidate.RequiredLevel > player->GetLevel() || candidate.ItemLevel > 22 ||
            candidate.Quality < 1 || candidate.Quality > 2 ||
            (candidate.AllowableClass != std::numeric_limits<uint32>::max() &&
                (candidate.AllowableClass & player->getClassMask()) == 0) ||
            (candidate.AllowableRace != std::numeric_limits<uint32>::max() &&
                (candidate.AllowableRace & player->getRaceMask()) == 0) ||
            (candidate.HasFlag2(ITEM_FLAG2_FACTION_HORDE) && player->GetTeamId(true) != TEAM_HORDE) ||
            (candidate.HasFlag2(ITEM_FLAG2_FACTION_ALLIANCE) && player->GetTeamId(true) != TEAM_ALLIANCE))
            continue;

        if (!bestAlternative || candidate.Quality > bestAlternative->Quality ||
            (candidate.Quality == bestAlternative->Quality && candidate.ItemLevel > bestAlternative->ItemLevel) ||
            (candidate.Quality == bestAlternative->Quality && candidate.ItemLevel == bestAlternative->ItemLevel && itemId < bestAlternative->ItemId))
            bestAlternative = &candidate;
    }

    return bestAlternative;
}

ItemTemplate const* FindPaladinRetributionTwoHandWeapon(Player* player)
{
    auto const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    if (!itemTemplates)
        return nullptr;

    ItemTemplate const* bestWeapon = nullptr;
    for (auto const& [itemId, candidate] : *itemTemplates)
    {
        bool const isPaladinWeaponSubclass = candidate.SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
            candidate.SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
            candidate.SubClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
            candidate.SubClass == ITEM_SUBCLASS_WEAPON_SWORD2;

        if (IsIgnoredGearItem(itemId) || candidate.HasFlag(ITEM_FLAG_DEPRECATED) || candidate.Class != ITEM_CLASS_WEAPON || candidate.InventoryType != INVTYPE_2HWEAPON ||
            !isPaladinWeaponSubclass || LevelBoostInternal::HasProfessionSkillRequirement(&candidate) || candidate.ItemLevel > 22 ||
            candidate.Quality < 1 || candidate.Quality > 2 ||
            !LevelBoostInternal::IsItemUsableForBoost(player, &candidate, player->GetLevel()))
            continue;

        if (!bestWeapon || candidate.Quality > bestWeapon->Quality ||
            (candidate.Quality == bestWeapon->Quality && candidate.ItemLevel > bestWeapon->ItemLevel) ||
            (candidate.Quality == bestWeapon->Quality && candidate.ItemLevel == bestWeapon->ItemLevel && itemId < bestWeapon->ItemId))
            bestWeapon = &candidate;
    }

    return bestWeapon;
}

bool IsRogueOffhandWeapon(ItemTemplate const* itemTemplate)
{
    if (!itemTemplate || itemTemplate->Class != ITEM_CLASS_WEAPON ||
        (itemTemplate->InventoryType != INVTYPE_WEAPON && itemTemplate->InventoryType != INVTYPE_WEAPONOFFHAND))
        return false;

    return itemTemplate->SubClass == ITEM_SUBCLASS_WEAPON_MACE ||
        itemTemplate->SubClass == ITEM_SUBCLASS_WEAPON_SWORD ||
        itemTemplate->SubClass == ITEM_SUBCLASS_WEAPON_FIST ||
        itemTemplate->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER;
}

ItemTemplate const* FindOffhandItem(Player* player, uint32 mainHandEntry)
{
    auto const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    if (!itemTemplates)
        return nullptr;

    ItemTemplate const* bestOffhandItem = nullptr;
    for (auto const& [itemId, candidate] : *itemTemplates)
    {
        bool const isRogueOffhandWeapon = player->getClass() == CLASS_ROGUE && IsRogueOffhandWeapon(&candidate);
        bool const isShield = player->getClass() != CLASS_ROGUE && candidate.Class == ITEM_CLASS_ARMOR &&
            candidate.SubClass == ITEM_SUBCLASS_ARMOR_SHIELD && candidate.InventoryType == INVTYPE_SHIELD;
        bool const isHoldable = player->getClass() != CLASS_ROGUE && candidate.InventoryType == INVTYPE_HOLDABLE;

        if (IsIgnoredGearItem(itemId) || candidate.HasFlag(ITEM_FLAG_DEPRECATED) || itemId == mainHandEntry || (!isRogueOffhandWeapon && !isShield && !isHoldable) ||
            LevelBoostInternal::HasProfessionSkillRequirement(&candidate) || candidate.ItemLevel > 22 ||
            candidate.Quality < 1 || candidate.Quality > 2 ||
            !LevelBoostInternal::IsItemUsableForBoost(player, &candidate, player->GetLevel()))
            continue;

        uint16 destination = 0;
        if (player->CanEquipNewItem(EQUIPMENT_SLOT_OFFHAND, destination, itemId, false) != EQUIP_ERR_OK)
            continue;

        if (!bestOffhandItem || candidate.Quality > bestOffhandItem->Quality ||
            (candidate.Quality == bestOffhandItem->Quality && candidate.ItemLevel > bestOffhandItem->ItemLevel) ||
            (candidate.Quality == bestOffhandItem->Quality && candidate.ItemLevel == bestOffhandItem->ItemLevel && itemId < bestOffhandItem->ItemId))
            bestOffhandItem = &candidate;
    }

    return bestOffhandItem;
}
}

void LevelBoost::LoadTalentContainer()
{
    talentTemplateList.clear();
    QueryResult result = WorldDatabase.Query("SELECT `player_class`, `player_spec`, `talent_id` FROM `mod_boost_talents`");

    if (!result)
    {
        LOG_WARN("server.loading", ">> Loaded 0 talent templates");
        return;
    }

    uint32 oldTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        TalentTemplate talent;
        talent.player_class = fields[0].Get<std::string>();
        talent.player_spec = fields[1].Get<std::string>();
        talent.talent_id = fields[2].Get<uint32>();
        talentTemplateList.push_back(talent);

        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} talent templates in {} ms.", count, GetMSTimeDiffToNow(oldTime));
}

void LevelBoost::LoadGlyphContainer()
{
    glyphTemplateList.clear();
    QueryResult result = WorldDatabase.Query("SELECT `player_class`, `player_spec`, `glyph_slot`, `glyph_id` FROM `mod_boost_glyphs`");

    if (!result)
    {
        LOG_WARN("server.loading", ">> Loaded 0 glyph templates");
        return;
    }

    uint32 oldTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        GlyphTemplate glyph;
        glyph.player_class = fields[0].Get<std::string>();
        glyph.player_spec = fields[1].Get<std::string>();
        glyph.glyph_slot = fields[2].Get<uint8>();
        glyph.glyph_id = fields[3].Get<uint32>();
        glyphTemplateList.push_back(glyph);

        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} glyph templates in {} ms.", count, GetMSTimeDiffToNow(oldTime));
}

void LevelBoost::LoadGearContainer()
{
    gearTemplateList.clear();
    QueryResult result = WorldDatabase.Query("SELECT `player_class`, `player_spec`, `player_race`, `equipment_slot`, `item_entry` FROM `mod_boost_gear`");

    if (!result)
    {
        LOG_WARN("server.loading", ">> Loaded 0 gear templates");
        return;
    }

    uint32 oldTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        GearTemplate gear;
        gear.player_class = fields[0].Get<std::string>();
        gear.player_spec = fields[1].Get<std::string>();
        gear.player_race = fields[2].Get<std::string>();
        gear.equipment_slot = fields[3].Get<uint8>();
        gear.item_entry = fields[4].Get<uint32>();
        gearTemplateList.push_back(gear);

        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} gear templates in {} ms.", count, GetMSTimeDiffToNow(oldTime));
}

void LevelBoost::LoadSpellContainer()
{
    classSpellsList.clear();
    QueryResult result = WorldDatabase.Query("SELECT `class_id`, `spell_id`, `required_level`, `required_spell_id` FROM `mod_boost_class_spells` ORDER BY `class_id`, `required_level`");

    if (!result)
    {
        LOG_WARN("server.loading", ">> Loaded 0 spell templates from mod_boost_class_spells; apply class_spells_table.sql and reload Level Boost templates.");
        return;
    }

    uint32 oldTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        ClassSpells spell;
        spell.class_id = fields[0].Get<uint8>();
        spell.spell_id = fields[1].Get<uint32>();
        spell.required_level = fields[2].Get<uint32>();
        spell.required_spell = fields[3].Get<uint32>();
        classSpellsList.push_back(spell);

        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} spell templates in {} ms.", count, GetMSTimeDiffToNow(oldTime));
}

void LevelBoost::LoadProficiencyContainer()
{
    classProficienciesList.clear();
    QueryResult result = WorldDatabase.Query("SELECT `class_id`, `spell_id`, `required_level` FROM `mod_boost_class_proficiencies` ORDER BY `class_id`");

    if (!result)
    {
        LOG_WARN("server.loading", ">> Loaded 0 proficiency templates");
        return;
    }

    uint32 oldTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        ClassProficiencies proficiencies;
        proficiencies.class_id = fields[0].Get<uint8>();
        proficiencies.spell_id = fields[1].Get<uint32>();
        proficiencies.required_level = fields[2].Get<uint32>();
        classProficienciesList.push_back(proficiencies);

        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} proficiency templates in {} ms.", count, GetMSTimeDiffToNow(oldTime));
}


bool LevelBoost::TemplateExists(Player* player, std::string const& playerSpec)
{
    std::string const playerClass = GetClassString(player);
    std::string const playerRace = GetRaceString(player);
    for (GearTemplate const& gearTemplate : gearTemplateList)
    {
        if (playerClass == gearTemplate.player_class &&
            playerSpec == gearTemplate.player_spec &&
            playerRace == gearTemplate.player_race)
            return true;
    }

    return false;
}


void LevelBoost::ApplyGearTemplate(Player* player, std::string const& playerSpec)
{
    std::string const playerClass = GetClassString(player);
    std::string const playerRace = GetRaceString(player);
    bool const isPaladinRetribution = playerClass == "Paladin" && playerSpec == "Retribution";
    bool const isRogue = playerClass == "Rogue";
    ItemTemplate const* retributionWeapon = isPaladinRetribution ? FindPaladinRetributionTwoHandWeapon(player) : nullptr;
    bool retributionWeaponEquipped = false;
    uint32 rogueMainHandEntry = 0;
    bool offhandEquipped = false;
    std::array<bool, EQUIPMENT_SLOT_END> processedSlots{};

    if (isPaladinRetribution && !retributionWeapon)
        LOG_WARN("module", "No eligible level {} two-handed weapon was found for Paladin Retribution {}.", player->GetLevel(), player->GetName());

    // Process the off-hand after the main hand so fallback selection can avoid duplicate weapons.
    for (uint8 pass = 0; pass < 2; ++pass)
    for (GearTemplate const& gearTemplate : gearTemplateList)
    {
        if (playerClass != gearTemplate.player_class ||
            playerSpec != gearTemplate.player_spec ||
            playerRace != gearTemplate.player_race)
            continue;

        bool const isOffhandSlot = gearTemplate.equipment_slot == EQUIPMENT_SLOT_OFFHAND;
        if (isOffhandSlot != (pass == 1))
            continue;

        if (gearTemplate.equipment_slot >= EQUIPMENT_SLOT_END)
        {
            LOG_WARN("module", "Ignoring invalid equipment slot {} in {} / {} / {} gear template.",
                gearTemplate.equipment_slot, playerClass, playerSpec, playerRace);
            continue;
        }

        if (processedSlots[gearTemplate.equipment_slot])
        {
            LOG_WARN("module", "Ignoring duplicate gear template row for slot {} in {} / {} / {}.",
                gearTemplate.equipment_slot, playerClass, playerSpec, playerRace);
            continue;
        }

        if (gearTemplate.equipment_slot == EQUIPMENT_SLOT_TRINKET1 ||
            gearTemplate.equipment_slot == EQUIPMENT_SLOT_TRINKET2)
            continue;

        if (isPaladinRetribution && gearTemplate.equipment_slot == EQUIPMENT_SLOT_OFFHAND && retributionWeapon)
            continue;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(gearTemplate.item_entry);
        if (IsIgnoredGearItem(gearTemplate.item_entry) ||
            (itemTemplate && itemTemplate->HasFlag(ITEM_FLAG_DEPRECATED)))
        {
            LOG_DEBUG("module", "Skipping ignored gear item {} for {} / {} / {} without replacement.",
                gearTemplate.item_entry, playerClass, playerSpec, playerRace);
            continue;
        }

        uint32 itemEntry = gearTemplate.item_entry;
        if (isPaladinRetribution && gearTemplate.equipment_slot == EQUIPMENT_SLOT_MAINHAND && retributionWeapon)
        {
            itemEntry = retributionWeapon->ItemId;
            itemTemplate = retributionWeapon;
            LOG_INFO("module", "Using two-handed weapon {} for Paladin Retribution {}.", itemEntry, player->GetName());
        }
        else if (!itemTemplate)
            continue;
        else if (isRogue && isOffhandSlot)
        {
            Item* mainHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            rogueMainHandEntry = mainHand ? mainHand->GetEntry() : 0;
            if (!IsRogueOffhandWeapon(itemTemplate) || itemEntry == rogueMainHandEntry)
            {
                ItemTemplate const* alternative = FindOffhandItem(player, rogueMainHandEntry);
                if (!alternative)
                {
                    LOG_WARN("module", "No valid distinct off-hand weapon could be found for Rogue {}.", player->GetName());
                    continue;
                }
                itemTemplate = alternative;
                itemEntry = alternative->ItemId;
                LOG_INFO("module", "Replacing invalid Rogue off-hand template item with weapon {} for {}.",
                    itemEntry, player->GetName());
            }
        }
        else if (LevelBoostInternal::HasProfessionSkillRequirement(itemTemplate))
        {
            ItemTemplate const* alternative = FindGearAlternative(player, itemTemplate);
            if (!alternative)
            {
                LOG_ERROR("module", "Gear item {} for {} / {} requires profession skill {} rank {}, and no suitable alternative was found.",
                    itemEntry, playerClass, playerSpec, itemTemplate->RequiredSkill, itemTemplate->RequiredSkillRank);
                continue;
            }

            LOG_WARN("module", "Replacing profession-gated gear item {} with {} for {} / {} / {}.",
                itemEntry, alternative->ItemId, playerClass, playerSpec, playerRace);
            itemEntry = alternative->ItemId;
            itemTemplate = alternative;
        }

        if (itemTemplate->RequiredLevel > player->GetLevel())
            continue;

        uint16 destination = 0;
        InventoryResult const equipResult = player->CanEquipNewItem(gearTemplate.equipment_slot, destination, itemEntry, false);
        if (equipResult != EQUIP_ERR_OK)
        {
            LOG_WARN("module", "Skipping gear item {} for {} in slot {} because it cannot be equipped (result {}).",
                itemEntry, player->GetName(), gearTemplate.equipment_slot, static_cast<uint32>(equipResult));
            continue;
        }

        Item* equippedItem = player->EquipNewItem(destination, itemEntry, true);
        if (!equippedItem)
            continue;
        processedSlots[gearTemplate.equipment_slot] = true;

        if (isRogue && gearTemplate.equipment_slot == EQUIPMENT_SLOT_MAINHAND)
            rogueMainHandEntry = itemEntry;
        if (isOffhandSlot)
            offhandEquipped = true;

        if (isPaladinRetribution && gearTemplate.equipment_slot == EQUIPMENT_SLOT_MAINHAND && retributionWeapon)
        {
            Item* equippedWeapon = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            retributionWeaponEquipped = equippedWeapon && equippedWeapon->GetEntry() == retributionWeapon->ItemId;
        }
    }

    if (isPaladinRetribution && retributionWeapon && !retributionWeaponEquipped)
    {
        player->EquipNewItem(EQUIPMENT_SLOT_MAINHAND, retributionWeapon->ItemId, true);
        LOG_INFO("module", "Equipped fallback two-handed weapon {} for Paladin Retribution {}.", retributionWeapon->ItemId, player->GetName());
    }

    if (!offhandEquipped && !(isPaladinRetribution && retributionWeapon))
    {
        if (ItemTemplate const* offhandItem = FindOffhandItem(player, rogueMainHandEntry))
        {
            uint16 destination = 0;
            if (player->CanEquipNewItem(EQUIPMENT_SLOT_OFFHAND, destination, offhandItem->ItemId, false) == EQUIP_ERR_OK &&
                player->EquipNewItem(destination, offhandItem->ItemId, true))
                LOG_INFO("module", "Equipped fallback off-hand item {} for {} {}.", offhandItem->ItemId, playerClass, player->GetName());
        }
        else
            LOG_WARN("module", "No eligible off-hand item could be equipped for {} {}.", playerClass, player->GetName());
    }
}


void LevelBoost::ApplyGlyphTemplate(Player* player, std::string const& playerSpec)
{
    if (!learnGlyphs || player->GetLevel() < 15)
        return;

    std::string const playerClass = GetClassString(player);
    uint32 const unlockedGlyphSlots = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
    for (GlyphTemplate const& glyphTemplate : glyphTemplateList)
    {
        if (playerClass != glyphTemplate.player_class || playerSpec != glyphTemplate.player_spec)
            continue;

        GlyphPropertiesEntry const* glyph = sGlyphPropertiesStore.LookupEntry(glyphTemplate.glyph_id);
        if (!glyph || !glyph->SpellId)
        {
            LOG_WARN("module", "Skipping invalid glyph template entry {} for {} / {}.",
                glyphTemplate.glyph_id, playerClass, playerSpec);
            continue;
        }

        auto isCompatibleUnlockedSlot = [player, unlockedGlyphSlots, glyph](uint8 slot)
        {
            if (slot >= MAX_GLYPH_SLOT_INDEX || !(unlockedGlyphSlots & (1u << slot)))
                return false;

            GlyphSlotEntry const* glyphSlot = sGlyphSlotStore.LookupEntry(player->GetGlyphSlot(slot));
            return glyphSlot && glyphSlot->TypeFlags == glyph->TypeFlags;
        };

        uint8 selectedSlot = glyphTemplate.glyph_slot;
        if (!isCompatibleUnlockedSlot(selectedSlot))
        {
            selectedSlot = MAX_GLYPH_SLOT_INDEX;
            for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
                if (isCompatibleUnlockedSlot(slot))
                {
                    selectedSlot = slot;
                    break;
                }
        }

        if (selectedSlot >= MAX_GLYPH_SLOT_INDEX)
        {
            LOG_WARN("module", "No unlocked compatible glyph slot for glyph {} ({}) for {} / {} at level {}.",
                glyphTemplate.glyph_id, glyph->SpellId, playerClass, playerSpec, player->GetLevel());
            continue;
        }

        if (uint32 oldGlyphId = player->GetGlyph(selectedSlot); oldGlyphId && oldGlyphId != glyphTemplate.glyph_id)
            if (GlyphPropertiesEntry const* oldGlyph = sGlyphPropertiesStore.LookupEntry(oldGlyphId))
            {
                player->RemoveAurasDueToSpell(oldGlyph->SpellId);
                player->SendLearnPacket(oldGlyph->SpellId, false);
            }

        player->SendLearnPacket(glyph->SpellId, true);
        player->CastSpell(player, glyph->SpellId,
            TriggerCastFlags(TRIGGERED_FULL_MASK & ~(TRIGGERED_IGNORE_SHAPESHIFT | TRIGGERED_IGNORE_CASTER_AURASTATE)));
        player->SetGlyph(selectedSlot, glyphTemplate.glyph_id, true);
        LOG_INFO("module", "Applied glyph {} to slot {} for {} / {}.",
            glyphTemplate.glyph_id, selectedSlot, playerClass, playerSpec);
    }
}


void LevelBoost::ApplyTalentTemplate(Player* player, std::string const& playerSpec)
{
    if (!learnTalents)
        return;

    std::string const playerClass = GetClassString(player);
    uint32 freeTalentPoints = player->GetFreeTalentPoints();
    for (TalentTemplate const& talentTemplate : talentTemplateList)
    {
        if (playerClass != talentTemplate.player_class || playerSpec != talentTemplate.player_spec)
            continue;

        uint32 const talentCost = GetTalentSpellCost(talentTemplate.talent_id);
        if (!talentCost || talentCost > freeTalentPoints)
            continue;

        if (player->addTalent(talentTemplate.talent_id, player->GetActiveSpecMask(), 0))
            freeTalentPoints -= talentCost;
    }

    player->SetFreeTalentPoints(freeTalentPoints);
}



void LevelBoost::ExtractGearTemplate(Player* player, std::string player_spec)
{
    WorldDatabase.Query("DELETE FROM mod_boost_gear WHERE player_class = '{}' AND player_spec = '{}' AND player_race = '{}'", GetClassString(player), player_spec, GetRaceString(player));

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (i != EQUIPMENT_SLOT_BODY)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                WorldDatabase.Execute("INSERT INTO mod_boost_gear (player_class, player_spec, player_race, equipment_slot, item_entry) VALUES ('{}', '{}', '{}', '{}', '{}')", GetClassString(player), player_spec, GetRaceString(player), i, item->GetEntry());
            }
        }
    }
}

void LevelBoost::ExtractTalentTemplate(Player* player, std::string player_spec)
{
    WorldDatabase.Query("DELETE FROM mod_boost_talents WHERE player_class = '{}' AND player_spec = '{}'", GetClassString(player), player_spec);

    QueryResult result = CharacterDatabase.Query("SELECT spell FROM character_talent WHERE guid = '{}' AND specMask = '{}'", player->GetGUID().GetCounter(), player->GetActiveSpecMask());

    if (!result)
        return;

    uint32 oldTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 spell = fields[0].Get<uint32>();
        WorldDatabase.Execute("INSERT INTO `mod_boost_talents` (`player_class`, `player_spec`, `talent_id`) VALUES ('{}', '{}', {})", GetClassString(player), player_spec, spell);
        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> Extracted {} talent templates in {} ms.", count, GetMSTimeDiffToNow(oldTime));
}

void LevelBoost::ExtractGlyphTemplate(Player* player, std::string player_spec)
{
    WorldDatabase.Query("DELETE FROM mod_boost_glyphs WHERE player_class = '{}' AND player_spec = '{}'", GetClassString(player), player_spec);

    QueryResult result = CharacterDatabase.Query("SELECT `glyph1`, `glyph2`, `glyph3`, `glyph4`, `glyph5`, `glyph6` FROM `character_glyphs` WHERE `guid`={} AND `talentGroup`={}", player->GetGUID().GetCounter(), player->GetActiveSpec());

    if (!result)
        return;

    Field* fields = result->Fetch();
    for (uint8 slot = 0; slot < 6; ++slot)
    {
        uint32 glyphId = fields[slot].Get<uint32>();
        if (glyphId == 0)
            continue;

        WorldDatabase.Execute("INSERT INTO `mod_boost_glyphs` (`player_class`, `player_spec`, `glyph_slot`, `glyph_id`) VALUES ('{}', '{}', {}, {})", GetClassString(player), player_spec, slot, glyphId);
    }
}

void LevelBoost::ExtractTemplates(Player* player, std::string player_spec)
{
    LOG_INFO("module", ">> Level Boost: Extracting gear template for {} {}...", GetClassString(player), player_spec);

    ExtractGearTemplate(player, player_spec);
    ExtractTalentTemplate(player, player_spec);
    ExtractGlyphTemplate(player, player_spec);
}
