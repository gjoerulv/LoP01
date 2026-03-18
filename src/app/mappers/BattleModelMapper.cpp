#include "app/mappers/BattleModelMapper.h"

#include <array>
#include <string>
#include <vector>

namespace app::mappers
{
    namespace
    {
        using ashvale::rendering::BattleActionView;
        using ashvale::rendering::BattleRenderModel;
        using ashvale::rendering::BattleTeam;
        using ashvale::rendering::BattleUnitView;

        bool IsUnitAlive(const gameplay::battle::BattleUnit& unit)
        {
            if (unit.category == gameplay::battle::UnitCategory::Generic)
            {
                return unit.life > 0;
            }

            return unit.hp > 0;
        }
    }

    ashvale::rendering::BattleRenderModel BattleModelMapper::Map(
        const gameplay::battle::BattleState& battle,
        const int selectedActionIndex,
        const int selectedTargetIndex,
        const std::string& statusText) const
    {
        BattleRenderModel model;
        model.battleTitle = "CTB Battle";
        // model.statusText = battle.LastActionText();
        model.statusText = statusText;

        const auto& units = battle.Units();
        const int activeIndex = battle.ActiveUnitIndex();

        int allyColumn = 0;
        int enemyColumn = 0;

        for (int i = 0; i < static_cast<int>(units.size()); ++i)
        {
            const auto& unit = units[i];
            const bool ally = unit.side == gameplay::battle::TeamSide::Allies;

            const float x = ally
                ? 44.0f + static_cast<float>(allyColumn % 3) * 196.0f
                : 672.0f + static_cast<float>(enemyColumn % 3) * 196.0f;
            const float y = 150.0f + static_cast<float>((ally ? allyColumn : enemyColumn) / 3) * 164.0f;

            if (ally)
            {
                ++allyColumn;
            }
            else
            {
                ++enemyColumn;
            }

            model.units.push_back(BattleUnitView{
                unit.name,
                ally ? BattleTeam::Allies : BattleTeam::Enemies,
                Rectangle{x, y, 184.0f, 146.0f},
                unit.hp,
                unit.stats.maxHp,
                unit.mp,
                unit.stats.maxMp,
                unit.life,
                i == activeIndex,
                IsUnitAlive(unit),
                i == selectedTargetIndex,
                unit.ko
                });
        }

        const std::vector<int> turnOrder = battle.UpcomingTurnOrder(6);
        for (const int index : turnOrder)
        {
            if (index >= 0 && index < static_cast<int>(units.size()))
            {
                model.turnOrder.push_back(units[index].name);
            }
        }

        static const std::array<std::string, 5> actionLabels = {
            "Attack", "Defend", "Wait", "Skill 1", "Skill 2"
        };

        for (int i = 0; i < static_cast<int>(actionLabels.size()); ++i)
        {
            model.actions.push_back(BattleActionView{
                actionLabels[i],
                true,
                i == selectedActionIndex
                });
        }

        if (selectedTargetIndex >= 0 && selectedTargetIndex < static_cast<int>(units.size()))
        {
            model.targetHint =
                "Target: " + units[selectedTargetIndex].name +
                "  (Up/Down to switch, Enter confirm)";
        }
        else
        {
            model.targetHint = "Choose action with Left/Right, confirm with Enter";
        }

        return model;
    }
}