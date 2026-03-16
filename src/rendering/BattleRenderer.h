#pragma once

#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    enum class BattleTeam
    {
        Allies,
        Enemies
    };

    struct BattleUnitView
    {
        std::string name;
        BattleTeam team = BattleTeam::Allies;
        Rectangle bounds{};

        int hp = 0;
        int maxHp = 0;
        int mp = 0;
        int maxMp = 0;
        int life = 1;

        bool active = false;
        bool targetable = false;
        bool selectedTarget = false;
        bool ko = false;
    };

    struct BattleActionView
    {
        std::string label;
        bool enabled = true;
        bool selected = false;
    };

    struct BattleRenderModel
    {
        std::string battleTitle = "Battle";
        std::string statusText;

        std::vector<BattleUnitView> units;
        std::vector<std::string> turnOrder;
        std::vector<BattleActionView> actions;

        std::string targetHint;
    };

    class BattleRenderer
    {
    public:
        void Draw(const RenderContext& context, const BattleRenderModel& model) const;

    private:
        void DrawUnitCard(const RenderContext& context, const BattleUnitView& unit) const;
    };
}