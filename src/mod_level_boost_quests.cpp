#include "mod_level_boost.h"
#include "mod_level_boost_internal.h"
#include "QuestDef.h"
#include <algorithm>
#include <unordered_set>

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
            if (!count || !LevelBoostInternal::IsItemUsableForBoost(player, itemTemplate, targetLevel))
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



void LevelBoost::CompleteClassQuests(Player* player, ClassQuestPlan const& plan)
{
    for (Quest const* quest : plan.invalidRewardQuests)
        LOG_WARN("module", "Marking class quest {} ({}) rewarded; one or more invalid fixed item rewards were skipped.", quest->GetQuestId(), quest->GetTitle());

    for (Quest const* quest : plan.unavailableChoiceRewardQuests)
        LOG_WARN("module", "Marking class quest {} ({}) rewarded for {}; no usable choice item was granted.", quest->GetQuestId(), quest->GetTitle(), player->GetName());

    uint32 completedQuests = 0;
    uint32 grantedRewardEntries = 0;
    uint32 highestQuestLevel = 0;
    std::unordered_set<uint32> processedRewardItems;

    for (PlannedClassQuest const& plannedQuest : plan.quests)
    {
        Quest const* quest = plannedQuest.quest;
        for (PlannedQuestReward const& reward : plannedQuest.rewards)
        {
            // Gear templates and multiple class quests can point at the same item. Grant each
            // reward item entry at most once per boost, and do not duplicate an item already owned.
            if (!processedRewardItems.insert(reward.item_id).second)
                continue;

            uint32 const ownedCount = player->GetItemCount(reward.item_id);
            if (ownedCount >= reward.count)
                continue;

            uint32 const missingCount = reward.count - ownedCount;
            ItemPosCountVec destination;
            uint32 noSpaceForCount = 0;
            InventoryResult const storageResult = player->CanStoreNewItem(
                NULL_BAG, NULL_SLOT, destination, reward.item_id, missingCount, &noSpaceForCount);
            if (storageResult == EQUIP_ERR_OK && noSpaceForCount == 0)
            {
                if (Item* item = player->StoreNewItem(destination, reward.item_id, true))
                {
                    player->SendNewItem(item, missingCount, true, false);
                    ++grantedRewardEntries;
                    continue;
                }
            }

            player->SendItemRetrievalMail(reward.item_id, missingCount);
            ++grantedRewardEntries;
            LOG_WARN("module", "Could not store class quest {} reward item {} x{} in {}'s inventory; sent it by mail.",
                quest->GetQuestId(), reward.item_id, missingCount, player->GetName());
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
