#include "mod_level_boost.h"
#include "mod_level_boost_internal.h"
#include "DBCStores.h"

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
            if (creature->IsQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if (!sLevelBoost->completeClassQuests && !sLevelBoost->learnSpells)
            {
                if (!player->PlayerTalkClass->GetQuestMenu().Empty())
                {
                    player->SendPreparedQuest(creature->GetGUID());
                    return true;
                }

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
        sLevelBoost->dualSpec = sConfigMgr->GetOption<bool>("DualSpec.Enable", false);
        sLevelBoost->learnSpells = sConfigMgr->GetOption<bool>("LearnSpells.Enable", true);
        sLevelBoost->LearnProficiencies = sConfigMgr->GetOption<bool>("LearnProficiencies.Enable", true);
        sLevelBoost->learnTalents = sConfigMgr->GetOption<bool>("LearnTalents.Enable", true);
        sLevelBoost->learnGlyphs = sConfigMgr->GetOption<bool>("LearnGlyphs.Enable", true);
        sLevelBoost->destroyGear = sConfigMgr->GetOption<bool>("DestroyGear.Enable", true);
        sLevelBoost->completeClassQuests = sConfigMgr->GetOption<bool>("ClassQuests.Enable", true);
        sLevelBoost->grantClassQuestItems = sConfigMgr->GetOption<bool>("ClassQuests.GrantItems", true);
        sLevelBoost->giveClassItems = sConfigMgr->GetOption<bool>("ClassItems.Enable", true);

        LOG_INFO("module", ">> Class spell learning is {}.", sLevelBoost->learnSpells ? "enabled" : "disabled");
        LOG_INFO("module", ">> Class quest completion is {}; quest item rewards are {}; class item grants are {}.",
            sLevelBoost->completeClassQuests ? "enabled" : "disabled",
            sLevelBoost->grantClassQuestItems ? "enabled" : "disabled",
            sLevelBoost->giveClassItems ? "enabled" : "disabled");
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
            else if (gear.equipment_slot == EQUIPMENT_SLOT_TRINKET1 || gear.equipment_slot == EQUIPMENT_SLOT_TRINKET2)
            {
                handler->PSendSysMessage("Trinket template row for item {} will be ignored; boost gear excludes trinkets.", gear.item_entry);
                ++warnings;
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
            if (LevelBoostInternal::HasProfessionSkillRequirement(item))
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
