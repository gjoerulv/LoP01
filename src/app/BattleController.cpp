#include "app/BattleController.h"

#include <algorithm>
#include <vector>

namespace app
{
    namespace
    {
        constexpr int kActionCount = 5;

        bool IsUnitAlive(const gameplay::battle::BattleUnit& unit)
        {
            if (unit.category == gameplay::battle::UnitCategory::Generic)
            {
                return unit.life > 0;
            }

            return unit.hp > 0;
        }

        gameplay::battle::BattleActionType ToActionType(const int index)
        {
            switch (index)
            {
            case 0: return gameplay::battle::BattleActionType::Attack;
            case 1: return gameplay::battle::BattleActionType::Defend;
            case 2: return gameplay::battle::BattleActionType::Wait;
            case 3: return gameplay::battle::BattleActionType::Skill1;
            default: return gameplay::battle::BattleActionType::Skill2;
            }
        }

        bool NeedsTarget(const gameplay::battle::BattleActionType action)
        {
            return action == gameplay::battle::BattleActionType::Attack ||
                action == gameplay::battle::BattleActionType::Skill1 ||
                action == gameplay::battle::BattleActionType::Skill2;
        }
    }

    BattleUpdateResult BattleController::Update(
        const input::InputState& input,
        const gameplay::battle::BattleState& battle,
        const BattleControllerState& state) const
    {
        BattleUpdateResult result{};
        result.state = state;

        if (battle.IsFinished())
        {
            return result;
        }

        if (input.selectPrev)
        {
            result.state.selectedActionIndex =
                (result.state.selectedActionIndex - 1 + kActionCount) % kActionCount;
        }

        if (input.selectNext)
        {
            result.state.selectedActionIndex =
                (result.state.selectedActionIndex + 1) % kActionCount;
        }

        const int activeIndex = battle.ActiveUnitIndex();
        const auto& units = battle.Units();

        std::vector<int> targetableIndices;
        if (activeIndex >= 0 && activeIndex < static_cast<int>(units.size()))
        {
            const auto actorSide = units[activeIndex].side;
            for (int i = 0; i < static_cast<int>(units.size()); ++i)
            {
                if (units[i].side != actorSide && IsUnitAlive(units[i]))
                {
                    targetableIndices.push_back(i);
                }
            }
        }

        if (!targetableIndices.empty())
        {
            if (result.state.selectedTargetIndex < 0 ||
                std::find(targetableIndices.begin(), targetableIndices.end(), result.state.selectedTargetIndex) == targetableIndices.end())
            {
                result.state.selectedTargetIndex = targetableIndices.front();
            }

            auto it = std::find(targetableIndices.begin(), targetableIndices.end(), result.state.selectedTargetIndex);
            int pos = static_cast<int>(it - targetableIndices.begin());

            if (input.targetPrev)
            {
                pos = (pos - 1 + static_cast<int>(targetableIndices.size())) % static_cast<int>(targetableIndices.size());
                result.state.selectedTargetIndex = targetableIndices[pos];
            }

            if (input.targetNext)
            {
                pos = (pos + 1) % static_cast<int>(targetableIndices.size());
                result.state.selectedTargetIndex = targetableIndices[pos];
            }
        }
        else
        {
            result.state.selectedTargetIndex = -1;
        }

        if (!input.confirm)
        {
            return result;
        }

        result.executeAction = true;
        result.action = ToActionType(result.state.selectedActionIndex);
        result.targetIndex = NeedsTarget(result.action) ? result.state.selectedTargetIndex : -1;

        return result;
    }
}