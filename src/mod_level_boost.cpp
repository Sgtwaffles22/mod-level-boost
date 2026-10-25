#include "mod_level_boost.h"
#include <algorithm>
#include <array>

void LevelBoost::LearnDualSpec(Player* player)
{
    if (!dualSpec)
        return;

    player->CastSpell(player, TALENT_SPECIALIZATION_SWITCHES, player->GetGUID());
    player->CastSpell(player, SECOND_TALENT_SPECIALIZATION, player->GetGUID());
}

void LevelBoost::DestroyGear(Player* player)
{
    if (!destroyGear)
        return;

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
    }
}

void LevelBoost::DestroyProjectiles(Player* player)
{
    if (player->getClass() != CLASS_HUNTER)
        return;

    Item* rangedWeapon = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!rangedWeapon)
        return;

    ItemTemplate const* rangedTemplate = rangedWeapon->GetTemplate();
    if (!rangedTemplate || rangedTemplate->Class != ITEM_CLASS_WEAPON)
        return;

    uint32 ammoEntry = 0;
    if (rangedTemplate->SubClass == ITEM_SUBCLASS_WEAPON_BOW ||
        rangedTemplate->SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW)
        ammoEntry = ITEM_BOOST_ARROWS;
    else if (rangedTemplate->SubClass == ITEM_SUBCLASS_WEAPON_GUN)
        ammoEntry = ITEM_BOOST_BULLETS;
    else
        return;

    std::array<uint32, 4> const managedAmmo = { ITEM_DEFAULT_ARROWS, ITEM_DEFAULT_BULLETS, ITEM_BOOST_ARROWS, ITEM_BOOST_BULLETS };
    for (uint32 itemEntry : managedAmmo)
    {
        uint32 const itemCount = player->GetItemCount(itemEntry);
        if (itemCount)
            player->DestroyItemCount(itemEntry, itemCount, true, true);
    }

    uint32 const ammoCount = ammoEntry == ITEM_BOOST_ARROWS ? ARROW_COUNT : BULLET_COUNT;
    if (player->AddItem(ammoEntry, ammoCount))
        player->SetAmmo(ammoEntry);
}

void LevelBoost::AddClassItems(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_SHAMAN:
            if (!player->HasItemCount(ITEM_SHAMAN_TOTEM_1))
                player->AddItem(ITEM_SHAMAN_TOTEM_1, 1);
            if (!player->HasItemCount(ITEM_SHAMAN_TOTEM_2))
                player->AddItem(ITEM_SHAMAN_TOTEM_2, 1);
            break;
    }
}

void LevelBoost::AddStarterBags(Player* player)
{
    if (!giveBags || !starterBagCount)
        return;

    ItemTemplate const* bagTemplate = sObjectMgr->GetItemTemplate(starterBagItem);
    if (!bagTemplate || bagTemplate->InventoryType != INVTYPE_BAG || !bagTemplate->ContainerSlots ||
        bagTemplate->ContainerSlots > MAX_BAG_SIZE)
    {
        LOG_ERROR("module", "Configured starter bag item {} is missing or is not a usable bag.", starterBagItem);
        return;
    }

    uint32 const maxEquippedBags = INVENTORY_SLOT_BAG_END - INVENTORY_SLOT_BAG_START;
    uint32 const targetBagCount = std::min(starterBagCount, maxEquippedBags);
    uint32 const existingBagCount = std::min(player->GetItemCount(starterBagItem), targetBagCount);
    uint32 bagsRemaining = targetBagCount - existingBagCount;
    if (starterBagCount > maxEquippedBags)
        LOG_WARN("module", "Configured starter bag count {} exceeds the four character bag slots; limiting the grant to {}.", starterBagCount, maxEquippedBags);
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END && bagsRemaining; ++slot)
    {
        if (player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            continue;

        if (player->EquipNewItem(slot, starterBagItem, true))
            --bagsRemaining;
        else
        {
            LOG_WARN("module", "Could not equip starter bag {} in bag slot {} for {}; skipped the extra bag.",
                starterBagItem, slot, player->GetName());
        }
    }
}

void LevelBoost::LearnSpellsForLevel(Player* player)
{
    if (!learnSpells)
    {
        LOG_WARN("module", "Class spell learning is disabled for {}; set LearnSpells.Enable = 1 and reload the configuration before boosting.", player->GetName());
        return;
    }

    uint32 eligibleSpells = 0;
    uint32 learnedSpells = 0;
    uint32 alreadyKnownSpells = 0;
    uint32 missingPrerequisites = 0;
    uint32 failedSpells = 0;

    for (ClassSpells const& classSpell : classSpellsList)
    {
        if (player->getClass() != classSpell.class_id || player->GetLevel() < classSpell.required_level)
            continue;

        ++eligibleSpells;
        if (classSpell.required_spell && !player->HasSpell(classSpell.required_spell))
        {
            ++missingPrerequisites;
            continue;
        }

        if (player->HasSpell(classSpell.spell_id))
        {
            ++alreadyKnownSpells;
            continue;
        }

        player->learnSpell(classSpell.spell_id);
        if (player->HasSpell(classSpell.spell_id))
            ++learnedSpells;
        else
            ++failedSpells;
    }

    LOG_INFO("module", ">> Class spell grant for {} at level {}: learned {} of {} eligible spells ({} already known, {} missing prerequisites, {} failed).",
        player->GetName(), player->GetLevel(), learnedSpells, eligibleSpells, alreadyKnownSpells, missingPrerequisites, failedSpells);
}

void LevelBoost::LearnProficienciesForLevel(Player* player)
{
    if (!LearnProficiencies)
        return;

    for (auto& classProficiency : classProficienciesList)
    {
        if (player->getClass() == classProficiency.class_id && player->GetLevel() >= classProficiency.required_level)
        {
            if (!player->HasSpell(classProficiency.spell_id))
            {
                player->learnSpell(classProficiency.spell_id);
            }
        }
    }
}

void LevelBoost::CreateHunterPet(Player* player, Creature* creature, uint32 entry)
{
    if (player->getClass() != CLASS_HUNTER)
        return;

    if (player->GetPet())
        return;

    Creature* tempCreature = creature->SummonCreature(entry, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 500);

    if (!tempCreature)
        return;

    Pet* hunterPet = player->CreateTamedPetFrom(tempCreature, 0);

    if (!hunterPet)
        return;

    tempCreature->setDeathState(DeathState::JustDied);
    tempCreature->RemoveCorpse();
    tempCreature->SetHealth(0);

    hunterPet->SetPower(POWER_HAPPINESS, 1048000);

    if (!hunterPet->InitStatsForLevel(player->GetLevel()))
    {
        LOG_ERROR("server", "Pet Create fail: No init stats for pet with entry {}", entry);
        delete hunterPet;
        return;
    }

    hunterPet->GetMap()->AddToMap(hunterPet->ToCreature());
    hunterPet->UpdateAllStats();
    hunterPet->SetLevel(player->GetLevel() - 1);

    player->SetMinion(hunterPet, true);

    hunterPet->SavePetToDB(PET_SAVE_AS_CURRENT);
    player->PetSpellInitialize();

}


void LevelBoost::HandleBoost(Player* player, Creature* creature, std::string const& playerSpec)
{
    if (oneTimeOnly && HasBoostClaim(player))
    {
        bool recheckedBoostRewards = false;
        if (completeClassQuests)
        {
            ClassQuestPlan const questPlan = BuildClassQuestPlan(player, GetTargetLevel());
            CompleteClassQuests(player, questPlan);
            recheckedBoostRewards = true;
        }

        if (learnSpells)
        {
            LearnSpellsForLevel(player);
            recheckedBoostRewards = true;
        }

        if (recheckedBoostRewards)
        {
            player->SaveToDB(false, false);
            player->GetSession()->SendAreaTriggerMessage("This character already claimed the boost. Eligible class quest rewards and missing class spells were checked.");
        }
        else
            player->GetSession()->SendAreaTriggerMessage("This character has already claimed a level boost.");
        return;
    }

    if (!ApplyFullTemplate(player, playerSpec))
        return;

    switch (player->getRace())
    {
    case RACE_NIGHTELF:
        CreateHunterPet(player, creature, 2031);
        break;
    case RACE_DWARF:
        CreateHunterPet(player, creature, 705);
        break;
    case RACE_DRAENEI:
        CreateHunterPet(player, creature, 16520);
        break;
    case RACE_ORC:
    case RACE_TROLL:
        CreateHunterPet(player, creature, 3098);
        break;
    case RACE_TAUREN:
        CreateHunterPet(player, creature, 2955);
        break;
    case RACE_BLOODELF:
        CreateHunterPet(player, creature, 15366);
        break;
    }

    LearnDualSpec(player);
    DestroyProjectiles(player);
    LearnSpellsForLevel(player);
    AddClassItems(player);
    player->UpdateSkillsToMaxSkillsForLevel();

    if (oneTimeOnly)
        RecordBoostClaim(player, GetTargetLevel());

    player->SaveToDB(false, false);
    CloseGossipMenuFor(player);
    player->SendTalentsInfoData(false);
}

std::string LevelBoost::GetClassString(Player* player)
{
    switch (player->getClass())
    {
    case CLASS_WARRIOR:
        return "Warrior";
        break;
    case CLASS_ROGUE:
        return "Rogue";
        break;
    case CLASS_SHAMAN:
        return "Shaman";
        break;
    case CLASS_HUNTER:
        return "Hunter";
        break;
    case CLASS_MAGE:
        return "Mage";
        break;
    case CLASS_WARLOCK:
        return "Warlock";
        break;
    case CLASS_PALADIN:
        return "Paladin";
        break;
    case CLASS_DRUID:
        return "Druid";
        break;
    case CLASS_PRIEST:
        return "Priest";
        break;
    }
    return "Unknown";
}

std::string LevelBoost::GetRaceString(Player* player)
{
    switch (player->getRace())
    {
    case RACE_HUMAN:
        return "Human";
        break;
    case RACE_DWARF:
        return "Dwarf";
        break;
    case RACE_NIGHTELF:
        return "Nightelf";
        break;
    case RACE_GNOME:
        return "Gnome";
        break;
    case RACE_DRAENEI:
        return "Draenei";
        break;
    case RACE_ORC:
        return "Orc";
        break;
    case RACE_UNDEAD_PLAYER:
        return "Undead";
        break;
    case RACE_TAUREN:
        return "Tauren";
        break;
    case RACE_TROLL:
        return "Troll";
        break;
    case RACE_BLOODELF:
        return "Bloodelf";
        break;
    }
    return "Unknown";
}


uint32 LevelBoost::GetTargetLevel() const
{
    uint32 maxPlayerLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    if (maxPlayerLevel > 255)
        maxPlayerLevel = 255;
    if (maxPlayerLevel == 0)
        maxPlayerLevel = 1;

    uint32 targetLevel = boostLevel;
    if (targetLevel == 0)
        targetLevel = 1;
    if (targetLevel > maxPlayerLevel)
        targetLevel = maxPlayerLevel;

    return targetLevel;
}

uint32 LevelBoost::GetMoneyToGrant(Player* player) const
{
    uint32 goldToGrant = startingGold;
    if (goldToGrant > 214748U)
        goldToGrant = 214748U;

    uint32 moneyToGrant = goldToGrant * 10000U;
    uint32 currentMoney = player->GetMoney();
    uint32 maxMoney = 2147483647U;
    if (currentMoney >= maxMoney)
        return 0;

    if (moneyToGrant > maxMoney - currentMoney)
        moneyToGrant = maxMoney - currentMoney;

    return moneyToGrant;
}

bool LevelBoost::HasBoostClaim(Player* player)
{
    if (!oneTimeOnly)
        return false;

    // Ignore stale claim rows when a deleted character's GUID is reused for a new character.
    QueryResult result = CharacterDatabase.Query(
        "SELECT 1 FROM mod_level_boost_claim WHERE guid = {} AND boosted_level <= {}",
        player->GetGUID().GetCounter(), player->GetLevel());
    return static_cast<bool>(result);
}

void LevelBoost::RecordBoostClaim(Player* player, uint32 targetLevel)
{
    CharacterDatabase.Execute(
        "INSERT INTO mod_level_boost_claim (guid, boosted_level, claim_time) VALUES ({}, {}, UNIX_TIMESTAMP()) "
        "ON DUPLICATE KEY UPDATE boosted_level = VALUES(boosted_level), claim_time = VALUES(claim_time)",
        player->GetGUID().GetCounter(), targetLevel);
}

bool LevelBoost::ApplyFullTemplate(Player* player, std::string const& playerSpec)
{
    if (!TemplateExists(player, playerSpec))
    {
        player->GetSession()->SendAreaTriggerMessage("There's no templates for %s specialization yet.", playerSpec.c_str());
        return false;
    }

    uint32 const targetLevel = GetTargetLevel();
    player->GiveLevel(static_cast<uint8>(targetLevel));
    // Learn weapon proficiencies (including Rogue Dual Wield) before equipment validation.
    LearnProficienciesForLevel(player);
    ClassQuestPlan const questPlan = BuildClassQuestPlan(player, targetLevel);
    DestroyGear(player);
    ApplyGearTemplate(player, playerSpec);
    AddStarterBags(player);
    ApplyGlyphTemplate(player, playerSpec);
    ApplyTalentTemplate(player, playerSpec);
    CompleteClassQuests(player, questPlan);

    uint32 const moneyToGrant = GetMoneyToGrant(player);
    if (moneyToGrant)
        player->ModifyMoney(static_cast<int32>(moneyToGrant));

    if (player->getPowerType() == POWER_MANA)
        player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));

    player->SetHealth(player->GetMaxHealth());
    return true;
}
