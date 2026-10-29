#ifndef DEF_LEVELBOOST_H
#define DEF_LEVELBOOST_H

#include "Player.h"
#include "Log.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Pet.h"
#include "Config.h"
#include "World.h"

#define ITEM_DEFAULT_ARROWS 2512
#define ITEM_DEFAULT_BULLETS 2516
#define ITEM_BOOST_ARROWS 2515
#define ITEM_BOOST_BULLETS 2519
#define ITEM_SHAMAN_TOTEM_1 5175
#define ITEM_SHAMAN_TOTEM_2 5176

const int ARROW_COUNT = 1000;
const int BULLET_COUNT = 1000;
const int BOOST_TEXT = 70000;

enum GossipId
{
    GOSSIP_MAIN_MENU = 0,
    GOSSIP_LEVEL_BOOST = 1,
    GOSSIP_TELEPORT_CITY = 3,
    GOSSIP_STAY_HERE = 4,
    GOSSIP_GEAR = 100,
    GOSSIP_RESET_TALENTS = 999
};

enum Spells
{
    TALENT_SPECIALIZATION_SWITCHES = 63680,
    SECOND_TALENT_SPECIALIZATION = 63624
};

struct PlannedQuestReward
{
    uint32 item_id;
    uint32 count;
};

struct PlannedClassQuest
{
    Quest const* quest;
    std::vector<PlannedQuestReward> rewards;
};

struct ClassQuestPlan
{
    std::vector<PlannedClassQuest> quests;
    std::vector<Quest const*> invalidRewardQuests;
    std::vector<Quest const*> unavailableChoiceRewardQuests;
};

struct ClassSpells
{
    uint8 class_id;
    uint32 spell_id;
    uint32 required_level;
    uint32 required_spell;
};

struct ClassProficiencies
{
    uint8 class_id;
    uint32 spell_id;
    uint32 required_level;
};

struct GearTemplate
{
    std::string player_class;
    std::string player_spec;
    std::string player_race;
    uint8 equipment_slot;
    uint32 item_entry;
    int32 random_property = 0;
};

struct TalentTemplate
{
    std::string player_class;
    std::string player_spec;
    uint32 talent_id;
};

struct GlyphTemplate
{
    std::string player_class;
    std::string player_spec;
    uint8 glyph_slot;
    uint32 glyph_id;
};

class LevelBoost
{
public:
    static LevelBoost* instance()
    {
        static LevelBoost instance;
        return &instance;
    }

    typedef std::vector<ClassSpells> ClassSpellsContainer;
    typedef std::vector<ClassProficiencies> ClassProficienciesContainer;
    typedef std::vector<GearTemplate> GearTemplateContainer;
    typedef std::vector<TalentTemplate> TalentTemplateContainer;
    typedef std::vector<GlyphTemplate> GlyphTemplateContainer;

    ClassSpellsContainer classSpellsList;
    ClassProficienciesContainer classProficienciesList;
    GearTemplateContainer gearTemplateList;
    TalentTemplateContainer talentTemplateList;
    GlyphTemplateContainer glyphTemplateList;

    bool levelBoost, oneTimeOnly, dualSpec, learnSpells, LearnProficiencies, learnTalents, learnGlyphs, destroyGear;
    bool completeClassQuests, grantClassQuestItems, giveClassItems, giveBags, useBisGear;
    uint32 boostLevel = 15;
    uint32 startingGold = 100;
    uint32 starterBagItem = 4240;
    uint32 starterBagCount = 4;

    void LoadTalentContainer();
    void LoadGlyphContainer();
    void LoadGearContainer();
    void LoadSpellContainer();
    void LoadProficiencyContainer();
    uint32 GetTargetLevel() const;
    uint32 GetMoneyToGrant(Player* player) const;
    bool HasBoostClaim(Player* player);
    bool HasAnyBoostClaim(Player* player);
    void RecordBoostClaim(Player* player, uint32 targetLevel);
    ClassQuestPlan BuildClassQuestPlan(Player* player, uint32 targetLevel) const;
    void CompleteClassQuests(Player* player, ClassQuestPlan const& plan);
    void LearnDualSpec(Player* player);
    void DestroyGear(Player* player);
    void DestroyProjectiles(Player* player);
    void AddClassItems(Player* player);
    void AddStarterBags(Player* player);
    void LearnSpellsForLevel(Player* player);
    void LearnProficienciesForLevel(Player* Player);
    bool HandleBoost(Player* player, Creature* creature, std::string const& playerSpec);
    void CreateHunterPet(Player* player, Creature* creature, uint32 entry);
    bool TemplateExists(Player* player, std::string const& playerSpec);
    bool ApplyFullTemplate(Player* player, std::string const& playerSpec);
    void ApplyGearTemplate(Player* player, std::string const& playerSpec);
    void ApplyBisGearTemplate(Player* player, std::string const& playerSpec);
    void ApplyTalentTemplate(Player* player, std::string const& playerSpec);
    void ApplyGlyphTemplate(Player* player, std::string const& playerSpec);
    void ExtractTemplates(Player* player, std::string player_spec);
    void ExtractGearTemplate(Player* player, std::string player_spec);
    void ExtractTalentTemplate(Player* player, std::string player_spec);
    void ExtractGlyphTemplate(Player* player, std::string player_spec);
    std::string GetClassString(Player* player);
    std::string GetRaceString(Player* player);
};

#define sLevelBoost LevelBoost::instance()

#endif
