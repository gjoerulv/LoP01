#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "app/BattleEventTextFormatter.h"

TEST_CASE("BattleEventTextFormatter renders information events from dedicated info text") {
    app::BattleEventTextFormatter formatter;

    const std::vector<gameplay::battle::BattleEvent> events = {
        gameplay::battle::BattleEvent{
            gameplay::battle::BattleEventType::Information,
            gameplay::battle::BattleActionType::Skill1,
            "Hero",
            "Slime",
            "Hero lacks MP, uses attack",
            0,
            gameplay::battle::TeamSide::Allies
        },
        gameplay::battle::BattleEvent{
            gameplay::battle::BattleEventType::AttackResolved,
            gameplay::battle::BattleActionType::Attack,
            "Hero",
            "Slime",
            "",
            7,
            gameplay::battle::TeamSide::Allies
        }
    };

    REQUIRE(formatter.FormatSummary(events) == "Hero lacks MP, uses attack | Hero attacks Slime for 7");
}
