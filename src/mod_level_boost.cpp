#include "mod_level_boost.h"
#include "DBCStores.h"
#include "QuestDef.h"
#include <algorithm>

namespace
{
struct SpecializationOption
{
    uint8 id;
    uint8 classId;
    char const* className;
    char const* specName;
};

SpecializationOption const SpecializationOptions[] =
{
    { 1, CLASS_WARRIOR, "Warrior", "Arms" },
    { 2, CLASS_WARRIOR, "Warrior", "Fury" },
    { 3, CLASS_WARRIOR, "Warrior", "Protection" },
    { 4, CLASS_ROGUE, "Rogue", "Assassination" },
    { 5, CLASS_ROGUE, "Rogue", "Combat" },
    { 6, CLASS_ROGUE, "Rogue", "Subtlety" },
    { 7, CLASS_SHAMAN, "Shaman", "Elemental" },
    { 8, CLASS_SHAMAN, "Shaman", "Enhancement" },
    { 9, CLASS_SHAMAN, "Shaman", "Restoration" },
    { 10, CLASS_HUNTER, "Hunter", "Beast Mastery" },
    { 11, CLASS_HUNTER, "Hunter", "Marksmanship" },
    { 12, CLASS_HUNTER, "Hunter", "Survival" },
    { 13, CLASS_MAGE, "Mage", "Arcane" },
    { 14, CLASS_MAGE, "Mage", "Fire" },
    { 15, CLASS_MAGE, "Mage", "Frost" },
    { 16, CLASS_WARLOCK, "Warlock", "Affliction" },
    { 17, CLASS_WARLOCK, "Warlock", "Demonology" },
    { 18, CLASS_WARLOCK, "Warlock", "Destruction" },
    { 19, CLASS_PALADIN, "Paladin", "Holy" },
    { 20, CLASS_PALADIN, "Paladin", "Protection" },
    { 21, CLASS_PALADIN, "Paladin", "Retribution" },
    { 22, CLASS_DRUID, "Druid", "Balance" },
    { 23, CLASS_DRUID, "Druid", "Feral" },
    { 24, CLASS_DRUID, "Druid", "Restoration" },
    { 25, CLASS_PRIEST, "Priest", "Discipline" },
    { 26, CLASS_PRIEST, "Priest", "Holy" },
    { 27, CLASS_PRIEST, "Priest", "Shadow" }
};

uint32 const SpecializationOptionCount = sizeof(SpecializationOptions) / sizeof(SpecializationOptions[0]);

SpecializationOption const* FindSpecializationOption(uint32 id)
{
    if (id == 0 || id > SpecializationOptionCount)
        return nullptr;

    SpecializationOption const& option = SpecializationOptions[id - 1];
    return option.id == id ? &option : nullptr;
}

uint32 GetClassMaskByName(std::string const& className)
{
    for (SpecializationOption const& option : SpecializationOptions)
        if (className == option.className)
            return 1U << (option.classId - 1);

    return 0;
}

uint32 GetRaceMaskByName(std::string const& raceName)
{
    struct RaceName
    {
        char const* name;
        uint8 raceId;
    };

    static RaceName const races[] =
    {
        { "Human", RACE_HUMAN },
        { "Orc", RACE_ORC },
        { "Dwarf", RACE_DWARF },
        { "Nightelf", RACE_NIGHTELF },
        { "Undead", RACE_UNDEAD_PLAYER },
        { "Tauren", RACE_TAUREN },
        { "Gnome", RACE_GNOME },
        { "Troll", RACE_TROLL },
        { "Bloodelf", RACE_BLOODELF },
        { "Draenei", RACE_DRAENEI }
    };

    for (RaceName const& race : races)
        if (raceName == race.name)
            return 1U << (race.raceId - 1);

    return 0;
}

bool IsChoiceItemUsableForBoost(Player* player, ItemTemplate const* itemTemplate, uint32 targetLevel)
{
    if (!itemTemplate || itemTemplate->RequiredLevel > targetLevel ||
        (itemTemplate->AllowableClass & player->getClassMask()) == 0 ||
        (itemTemplate->AllowableRace & player->getRaceMask()) == 0)
        return false;

    if (itemTemplate->HasFlag2(ITEM_FLAG2_FACTION_HORDE) && player->GetTeamId(true) != TEAM_HORDE)
        return false;

    if (itemTemplate->HasFlag2(ITEM_FLAG2_FACTION_ALLIANCE) && player->GetTeamId(true) != TEAM_ALLIANCE)
        return false;

    return true;
}

bool HasProfessionSkillRequirement(ItemTemplate const* itemTemplate)
{
    return itemTemplate && (itemTemplate->RequiredSkill || itemTemplate->RequiredSkillRank);
}

ItemTemplate const* FindProfessionFreeAlternative(Player* player, ItemTemplate const* original)
{
    if (!original)
        return nullptr;

    auto const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    if (!itemTemplates)
        return nullptr;

    ItemTemplate const* bestAlternative = nullptr;
    for (auto const& [itemId, candidate] : *itemTemplates)
    {
        if (candidate.Class != original->Class || candidate.SubClass != original->SubClass ||
            candidate.InventoryType != original->InventoryType || HasProfessionSkillRequirement(&candidate) ||
            candidate.RequiredLevel > player->GetLevel() || candidate.ItemLevel > 22 ||
            candidate.Quality < 1 || candidate.Quality > 2 ||
            (candidate.AllowableClass != -1 && (candidate.AllowableClass & player->getClassMask()) == 0) ||
            (candidate.AllowableRace != -1 && (candidate.AllowableRace & player->getRaceMask()) == 0) ||
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

    if (player->HasItemCount(ITEM_DEFAULT_ARROWS))
    {
        player->DestroyItemCount(ITEM_DEFAULT_ARROWS, player->GetItemCount(ITEM_DEFAULT_ARROWS), true, true);
        player->AddItem(ITEM_BOOST_ARROWS, ARROW_COUNT);
        player->SetAmmo(ITEM_BOOST_ARROWS);
    }
    else
    {
        player->DestroyItemCount(ITEM_DEFAULT_BULLETS, player->GetItemCount(ITEM_DEFAULT_BULLETS), true, true);
        player->AddItem(ITEM_BOOST_BULLETS, BULLET_COUNT);
        player->SetAmmo(ITEM_BOOST_BULLETS);
    }
}

void LevelBoost::AddClassItems(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_SHAMAN:
            player->AddItem(ITEM_SHAMAN_TOTEM_1, 1);
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
    uint32 bagsRemaining = std::min(starterBagCount, maxEquippedBags);
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
            player->SendItemRetrievalMail(starterBagItem, 1);
            --bagsRemaining;
            LOG_WARN("module", "Could not equip starter bag {} in bag slot {} for {}; sent it by mail.",
                starterBagItem, slot, player->GetName());
        }
    }

    while (bagsRemaining)
    {
        player->SendItemRetrievalMail(starterBagItem, 1);
        --bagsRemaining;
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
    LearnProficienciesForLevel(player);
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

    QueryResult result = CharacterDatabase.Query(
        "SELECT 1 FROM mod_level_boost_claim WHERE guid = {}",
        player->GetGUID().GetCounter());
    return static_cast<bool>(result);
}

void LevelBoost::RecordBoostClaim(Player* player, uint32 targetLevel)
{
    CharacterDatabase.Execute(
        "INSERT IGNORE INTO mod_level_boost_claim (guid, boosted_level, claim_time) VALUES ({}, {}, UNIX_TIMESTAMP())",
        player->GetGUID().GetCounter(), targetLevel);
}

ClassQuestPlan LevelBoost::BuildClassQuestPlan(Player* player, uint32 targetLevel) const
{
    ClassQuestPlan plan;
    if (!completeClassQuests)
        return plan;

    uint32 const playerClassMask = player->getClassMask();
    uint32 const playerRaceMask = player->getRaceMask();

    for (auto const& questEntry : sObjectMgr->GetQuestTemplates())
    {
        Quest const* quest = questEntry.second;
        if (!quest)
            continue;

        uint32 const requiredClasses = quest->GetRequiredClasses();
        if (!requiredClasses || !(requiredClasses & playerClassMask))
            continue;

        if (quest->GetMinLevel() > targetLevel)
            continue;

        int32 const questLevel = quest->GetQuestLevel();
        if (questLevel > 0 && static_cast<uint32>(questLevel) > targetLevel)
            continue;

        uint32 const allowableRaces = quest->GetAllowableRaces();
        if (allowableRaces && !(allowableRaces & playerRaceMask))
            continue;

        // Spell reward quests can have race-specific availability conditions even when their
        // allowable-race mask is empty (for example, racial versions of class spell quests).
        // Respect those conditions so a boost cannot learn another race's reward spell.
        if ((quest->GetRewSpell() || quest->GetRewSpellCast()) && !player->SatisfyQuestConditions(quest, false))
            continue;

        if (quest->IsRepeatable() || quest->IsDailyOrWeekly() || quest->IsMonthly() ||
            quest->IsSeasonal() || quest->IsPVPQuest() || quest->GetTimeAllowed() > 0)
            continue;

        uint32 const questId = quest->GetQuestId();
        if (player->IsQuestRewarded(questId))
            continue;

        PlannedClassQuest plannedQuest;
        plannedQuest.quest = quest;
        bool invalidReward = false;

        for (uint8 rewardIndex = 0; rewardIndex < QUEST_REWARDS_COUNT; ++rewardIndex)
        {
            uint32 const itemId = quest->RewardItemId[rewardIndex];
            uint32 const count = quest->RewardItemIdCount[rewardIndex];
            if (!itemId || !count)
                continue;

            if (!sObjectMgr->GetItemTemplate(itemId))
            {
                invalidReward = true;
                continue;
            }

            plannedQuest.rewards.push_back({ itemId, count });
        }

        if (invalidReward)
            plan.invalidRewardQuests.push_back(quest);

        bool hasChoiceReward = false;
        bool selectedChoiceReward = false;
        for (uint8 rewardIndex = 0; rewardIndex < QUEST_REWARD_CHOICES_COUNT; ++rewardIndex)
        {
            uint32 const itemId = quest->RewardChoiceItemId[rewardIndex];
            uint32 const count = quest->RewardChoiceItemCount[rewardIndex];
            if (!itemId)
                continue;

            hasChoiceReward = true;
            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
            if (!count || !IsChoiceItemUsableForBoost(player, itemTemplate, targetLevel))
                continue;

            plannedQuest.rewards.push_back({ itemId, count });
            selectedChoiceReward = true;
            break;
        }

        if (hasChoiceReward && !selectedChoiceReward)
        {
            plan.unavailableChoiceRewardQuests.push_back(quest);
        }

        plan.quests.push_back(plannedQuest);
    }

    return plan;
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
    for (GearTemplate const& gearTemplate : gearTemplateList)
    {
        if (playerClass != gearTemplate.player_class ||
            playerSpec != gearTemplate.player_spec ||
            playerRace != gearTemplate.player_race)
            continue;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(gearTemplate.item_entry);
        if (!itemTemplate)
            continue;

        uint32 itemEntry = gearTemplate.item_entry;
        if (HasProfessionSkillRequirement(itemTemplate))
        {
            ItemTemplate const* alternative = FindProfessionFreeAlternative(player, itemTemplate);
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

        player->EquipNewItem(gearTemplate.equipment_slot, itemEntry, true);
    }
}


void LevelBoost::ApplyGlyphTemplate(Player* player, std::string const& playerSpec)
{
    if (!learnGlyphs || player->GetLevel() < 15)
        return;

    std::string const playerClass = GetClassString(player);
    for (GlyphTemplate const& glyphTemplate : glyphTemplateList)
        if (playerClass == glyphTemplate.player_class && playerSpec == glyphTemplate.player_spec)
            player->SetGlyph(glyphTemplate.glyph_slot, glyphTemplate.glyph_id, true);
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


void LevelBoost::CompleteClassQuests(Player* player, ClassQuestPlan const& plan)
{
    for (Quest const* quest : plan.invalidRewardQuests)
        LOG_WARN("module", "Marking class quest {} ({}) rewarded; one or more invalid fixed item rewards were skipped.", quest->GetQuestId(), quest->GetTitle());

    for (Quest const* quest : plan.unavailableChoiceRewardQuests)
        LOG_WARN("module", "Marking class quest {} ({}) rewarded for {}; no usable choice item was granted.", quest->GetQuestId(), quest->GetTitle(), player->GetName());

    uint32 completedQuests = 0;
    uint32 grantedRewardEntries = 0;
    uint32 highestQuestLevel = 0;

    for (PlannedClassQuest const& plannedQuest : plan.quests)
    {
        Quest const* quest = plannedQuest.quest;
        for (PlannedQuestReward const& reward : plannedQuest.rewards)
        {
            ItemPosCountVec destination;
            uint32 noSpaceForCount = 0;
            InventoryResult const storageResult = player->CanStoreNewItem(
                NULL_BAG, NULL_SLOT, destination, reward.item_id, reward.count, &noSpaceForCount);
            if (storageResult == EQUIP_ERR_OK && noSpaceForCount == 0)
            {
                if (Item* item = player->StoreNewItem(destination, reward.item_id, true))
                {
                    player->SendNewItem(item, reward.count, true, false);
                    ++grantedRewardEntries;
                    continue;
                }
            }

            player->SendItemRetrievalMail(reward.item_id, reward.count);
            ++grantedRewardEntries;
            LOG_WARN("module", "Could not store class quest {} reward item {} x{} in {}'s inventory; sent it by mail.",
                quest->GetQuestId(), reward.item_id, reward.count, player->GetName());
        }

        uint16 const questLogSlot = player->FindQuestSlot(quest->GetQuestId());
        if (questLogSlot < MAX_QUEST_LOG_SIZE)
            player->SetQuestSlot(questLogSlot, 0);

        if (player->GetQuestStatus(quest->GetQuestId()) != QUEST_STATUS_NONE)
            player->RemoveActiveQuest(quest->GetQuestId(), false);

        player->learnQuestRewardedSpells(quest);
        player->SetRewardedQuest(quest->GetQuestId());
        player->SendQuestUpdate(quest->GetQuestId());
        ++completedQuests;

        if (quest->GetQuestLevel() > 0)
            highestQuestLevel = std::max(highestQuestLevel, static_cast<uint32>(quest->GetQuestLevel()));
    }

    if (completedQuests)
        LOG_INFO("module", ">> Marked {} eligible class quests through level {} as rewarded for {} and granted {} quest item entries.",
            completedQuests, highestQuestLevel, player->GetName(), grantedRewardEntries);
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


class BoostNPC_Template : public CreatureScript
{
public:
    BoostNPC_Template() : CreatureScript("BoostNPC_Template") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);

        if (!sLevelBoost->levelBoost)
            return false;

        if (sLevelBoost->HasBoostClaim(player))
        {
            if (!sLevelBoost->completeClassQuests && !sLevelBoost->learnSpells)
            {
                player->GetSession()->SendAreaTriggerMessage("This character has already claimed a level boost.");
                return false;
            }

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Update eligible class quests and missing spells", GOSSIP_SENDER_MAIN, GOSSIP_RECHECK_BOOST);
            SendGossipMenuFor(player, BOOST_TEXT, creature->GetGUID());
            return true;
        }

        if (player->GetLevel() != 1)
            return false;

        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Tell me about the level boost...", GOSSIP_SENDER_MAIN, GOSSIP_LEVEL_BOOST);

        switch (player->getRace())
        {
        case RACE_HUMAN:
            SendGossipMenuFor(player, 50016, creature->GetGUID());
            break;
        case RACE_NIGHTELF:
            SendGossipMenuFor(player, 4936, creature->GetGUID());
            break;
        case RACE_DWARF:
        case RACE_GNOME:
            SendGossipMenuFor(player, 4937, creature->GetGUID());
            break;
        case RACE_DRAENEI:
            SendGossipMenuFor(player, 8667, creature->GetGUID());
            break;
        case RACE_ORC:
        case RACE_TROLL:
            SendGossipMenuFor(player, 3583, creature->GetGUID());
            break;
        case RACE_TAUREN:
            SendGossipMenuFor(player, 4935, creature->GetGUID());
            break;
        case RACE_BLOODELF:
            SendGossipMenuFor(player, 16703, creature->GetGUID());
            break;
        case RACE_UNDEAD_PLAYER:
            SendGossipMenuFor(player, 938, creature->GetGUID());
            break;
        default:
            return false;
        }

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (!sLevelBoost->levelBoost || sender != GOSSIP_SENDER_MAIN)
            return false;

        if (sLevelBoost->HasBoostClaim(player))
        {
            if (action == GOSSIP_RECHECK_BOOST)
            {
                sLevelBoost->HandleBoost(player, creature, "");
                return true;
            }

            if (action == GOSSIP_MAIN_MENU)
                return OnGossipHello(player, creature);

            return false;
        }

        if (player->GetLevel() != 1)
            return false;

        if (action == GOSSIP_MAIN_MENU)
            return OnGossipHello(player, creature);

        if (action == GOSSIP_LEVEL_BOOST)
        {
            ClearGossipMenuFor(player);
            AddOptionsForClass(player);
            SendGossipMenuFor(player, BOOST_TEXT, creature->GetGUID());
            return true;
        }

        if (action > GOSSIP_GEAR && action <= GOSSIP_GEAR + SpecializationOptionCount)
        {
            SpecializationOption const* option = FindSpecializationOption(action - GOSSIP_GEAR);
            if (!option || option->classId != player->getClass())
                return false;

            sLevelBoost->HandleBoost(player, creature, option->specName);
            return true;
        }

        return false;
    }


    void AddOptionsForClass(Player* player)
    {
        for (SpecializationOption const& option : SpecializationOptions)
        {
            if (option.classId != player->getClass())
                continue;

            std::string text = "I would like to play ";
            text += option.specName;
            text += ".";
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, text, GOSSIP_SENDER_MAIN, GOSSIP_GEAR + option.id);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back...", GOSSIP_SENDER_MAIN, GOSSIP_MAIN_MENU);
    }

};

class BoostNPC_World : public WorldScript
{
public:
    BoostNPC_World(): WorldScript("BoostNPC_World") { }

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        LoadConfig();
    }

    void OnStartup() override
    {
        LOG_INFO("module", ">> Loading Spell Container...");
        sLevelBoost->LoadSpellContainer();

        LOG_INFO("module", ">> Loading Proficiency Container...");
        sLevelBoost->LoadProficiencyContainer();

        LOG_INFO("module", ">> Loading Gear Container...");
        sLevelBoost->LoadGearContainer();

        LOG_INFO("module", ">> Loading Talent Container...");
        sLevelBoost->LoadTalentContainer();

        LOG_INFO("module", ">> Loading Glyph Container...");
        sLevelBoost->LoadGlyphContainer();

    }

    static void LoadConfig()
    {
        uint32 oldTime = getMSTime();

        sLevelBoost->levelBoost = sConfigMgr->GetOption<bool>("LevelBoost.Enable", true);
        sLevelBoost->boostLevel = sConfigMgr->GetOption<uint32>("LevelBoost.Level", 15);
        sLevelBoost->startingGold = sConfigMgr->GetOption<uint32>("LevelBoost.Gold", 100);
        sLevelBoost->giveBags = sConfigMgr->GetOption<bool>("Bags.Enable", true);
        sLevelBoost->starterBagItem = sConfigMgr->GetOption<uint32>("Bags.Item", 4240);
        sLevelBoost->starterBagCount = sConfigMgr->GetOption<uint32>("Bags.Count", 4);
        sLevelBoost->oneTimeOnly = sConfigMgr->GetOption<bool>("LevelBoost.OneTimeOnly", true);
        sLevelBoost->dualSpec = sConfigMgr->GetOption<bool>("DualSpec.Enable", true);
        sLevelBoost->learnSpells = sConfigMgr->GetOption<bool>("LearnSpells.Enable", true);
        sLevelBoost->LearnProficiencies = sConfigMgr->GetOption<bool>("LearnProficiencies.Enable", true);
        sLevelBoost->learnTalents = sConfigMgr->GetOption<bool>("LearnTalents.Enable", true);
        sLevelBoost->learnGlyphs = sConfigMgr->GetOption<bool>("LearnGlyphs.Enable", true);
        sLevelBoost->destroyGear = sConfigMgr->GetOption<bool>("DestroyGear.Enable", true);
        sLevelBoost->completeClassQuests = sConfigMgr->GetOption<bool>("ClassQuests.Enable", true);

        LOG_INFO("module", ">> Class spell learning is {}.", sLevelBoost->learnSpells ? "enabled" : "disabled");
        LOG_INFO("module", ">> Loaded Level Boost Config in {} ms.", GetMSTimeDiffToNow(oldTime));
    }
};

using namespace Acore::ChatCommands;

class BoostNPC_Command : public CommandScript
{
public:
    BoostNPC_Command() : CommandScript("BoostNPC_Command") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable LevelBoostTable =
        {
            { "reload", HandleTableReload, SEC_ADMINISTRATOR, Console::No },
            { "validate", HandleValidateTemplates, SEC_ADMINISTRATOR, Console::Yes },
            { "extract", HandleExtractTemplate, SEC_ADMINISTRATOR, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "levelboost", LevelBoostTable }
        };

        return commandTable;
    }

    static bool HandleTableReload(ChatHandler* handler)
    {
        LOG_INFO("module", ">> Reloading templates for Level Boost...");
        sLevelBoost->LoadSpellContainer();
        sLevelBoost->LoadProficiencyContainer();
        sLevelBoost->LoadGearContainer();
        sLevelBoost->LoadTalentContainer();
        sLevelBoost->LoadGlyphContainer();
        handler->SendGlobalSysMessage("Level Boost templates reloaded.");
        return true;
    }


    static bool HandleValidateTemplates(ChatHandler* handler)
    {
        uint32 errors = 0;
        uint32 warnings = 0;
        uint32 const targetLevel = sLevelBoost->GetTargetLevel();

        handler->SendSysMessage("Validating Level Boost templates...");
        for (SpecializationOption const& option : SpecializationOptions)
        {
            bool hasGear = false;
            bool hasTalents = false;
            bool hasGlyphs = false;

            for (GearTemplate const& gear : sLevelBoost->gearTemplateList)
                hasGear = hasGear || (gear.player_class == option.className && gear.player_spec == option.specName);
            for (TalentTemplate const& talent : sLevelBoost->talentTemplateList)
                hasTalents = hasTalents || (talent.player_class == option.className && talent.player_spec == option.specName);
            for (GlyphTemplate const& glyph : sLevelBoost->glyphTemplateList)
                hasGlyphs = hasGlyphs || (glyph.player_class == option.className && glyph.player_spec == option.specName);

            if (!hasGear)
            {
                handler->PSendSysMessage("Missing gear template: {} / {}", option.className, option.specName);
                ++errors;
            }
            if (!hasTalents)
            {
                handler->PSendSysMessage("No talent template: {} / {}", option.className, option.specName);
                ++warnings;
            }
            if (!hasGlyphs)
            {
                handler->PSendSysMessage("No glyph template: {} / {}", option.className, option.specName);
                ++warnings;
            }
        }

        for (GearTemplate const& gear : sLevelBoost->gearTemplateList)
        {
            ItemTemplate const* item = sObjectMgr->GetItemTemplate(gear.item_entry);
            if (!item)
            {
                handler->PSendSysMessage("Invalid gear item {} for {} / {} / {}", gear.item_entry, gear.player_class, gear.player_spec, gear.player_race);
                ++errors;
                continue;
            }

            if (gear.equipment_slot >= EQUIPMENT_SLOT_END)
            {
                handler->PSendSysMessage("Invalid equipment slot {} for item {} ({} / {})", gear.equipment_slot, gear.item_entry, gear.player_class, gear.player_spec);
                ++errors;
            }

            uint32 const classMask = GetClassMaskByName(gear.player_class);
            uint32 const raceMask = GetRaceMaskByName(gear.player_race);
            if (!classMask || (static_cast<uint32>(item->AllowableClass) & classMask) == 0)
            {
                handler->PSendSysMessage("Item {} cannot be used by class {}.", gear.item_entry, gear.player_class);
                ++errors;
            }
            if (!raceMask || (static_cast<uint32>(item->AllowableRace) & raceMask) == 0)
            {
                handler->PSendSysMessage("Item {} cannot be used by race {}.", gear.item_entry, gear.player_race);
                ++errors;
            }
            if (item->RequiredLevel > targetLevel)
            {
                handler->PSendSysMessage("Item {} requires level {}, above the configured boost level {}.", gear.item_entry, item->RequiredLevel, targetLevel);
                ++warnings;
            }
            if (HasProfessionSkillRequirement(item))
            {
                handler->PSendSysMessage("Item {} requires profession skill {} rank {}; the boost will substitute matching gear when possible.",
                    gear.item_entry, item->RequiredSkill, item->RequiredSkillRank);
                ++warnings;
            }
        }

        for (TalentTemplate const& talent : sLevelBoost->talentTemplateList)
            if (!GetTalentSpellCost(talent.talent_id))
            {
                handler->PSendSysMessage("Invalid talent spell {} for {} / {}.", talent.talent_id, talent.player_class, talent.player_spec);
                ++errors;
            }

        for (GlyphTemplate const& glyph : sLevelBoost->glyphTemplateList)
            if (glyph.glyph_slot >= 6 || !sGlyphPropertiesStore.LookupEntry(glyph.glyph_id))
            {
                handler->PSendSysMessage("Invalid glyph {} in slot {} for {} / {}.", glyph.glyph_id, glyph.glyph_slot, glyph.player_class, glyph.player_spec);
                ++errors;
            }

        if (sLevelBoost->giveBags && sLevelBoost->starterBagCount)
        {
            ItemTemplate const* bag = sObjectMgr->GetItemTemplate(sLevelBoost->starterBagItem);
            if (!bag || bag->InventoryType != INVTYPE_BAG || !bag->ContainerSlots || bag->ContainerSlots > MAX_BAG_SIZE)
            {
                handler->PSendSysMessage("Configured starter bag item {} is missing or is not a usable bag.", sLevelBoost->starterBagItem);
                ++errors;
            }
            if (sLevelBoost->starterBagCount > INVENTORY_SLOT_BAG_END - INVENTORY_SLOT_BAG_START)
            {
                handler->PSendSysMessage("Configured bag count {} exceeds the {} character bag slots; the count will be clamped.",
                    sLevelBoost->starterBagCount, INVENTORY_SLOT_BAG_END - INVENTORY_SLOT_BAG_START);
                ++warnings;
            }
        }

        handler->PSendSysMessage("Level Boost validation found {} error(s) and {} warning(s).", errors, warnings);
        return true;
    }

    static bool HandleExtractTemplate(ChatHandler* handler, std::string player_spec)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
        {
            LOG_INFO("module", ">> Level Boost: No player selected.");
            return false;
        }

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        player->SaveToDB(trans, false, false);
        CharacterDatabase.CommitTransaction(trans);

        sLevelBoost->ExtractTemplates(player, player_spec);
        handler->SendSysMessage("Successfully extracted template.");

        return true;
    }
};

void AddSC_BoostNPC()
{
    new BoostNPC_Template();
    new BoostNPC_World();
    new BoostNPC_Command();
}
