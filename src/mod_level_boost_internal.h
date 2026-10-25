#ifndef MOD_LEVEL_BOOST_INTERNAL_H
#define MOD_LEVEL_BOOST_INTERNAL_H

#include "mod_level_boost.h"

namespace LevelBoostInternal
{
inline bool IsItemUsableForBoost(Player* player, ItemTemplate const* itemTemplate, uint32 targetLevel)
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

inline bool HasProfessionSkillRequirement(ItemTemplate const* itemTemplate)
{
    return itemTemplate && (itemTemplate->RequiredSkill || itemTemplate->RequiredSkillRank);
}

}

#endif
