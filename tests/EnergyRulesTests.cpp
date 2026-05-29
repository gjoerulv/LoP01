#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/EnergyRules.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"

using namespace gameplay;

namespace {

data::UnitDefinition MakeHero(const std::string& id, int agility) {
    data::UnitDefinition def;
    def.id = id;
    def.name = id;
    def.category = data::UnitDefinitionCategory::Hero;
    def.stats.agility = agility;
    return def;
}

// Session with the given heroes owned; the first is moved into the active party,
// the rest stay in reserve. The unit catalog is wired so the daily-energy
// agility term can resolve. Returns the session by value.
GameSession MakeSessionWithParty(const std::vector<data::UnitDefinition>& heroes) {
    GameSession session;

    std::vector<std::string> leaderCapable;
    for (const auto& h : heroes) {
        leaderCapable.push_back(h.id);
    }
    session.SetLeaderCapableUnitIds(leaderCapable);
    session.SetUnitCatalog(heroes);

    bool first = true;
    for (const auto& h : heroes) {
        REQUIRE(session.AddOwnedUnit(h.id, 1));
        if (first) {
            REQUIRE(session.TryAddUnitToActiveParty(h.id)); // active
            first = false;
        }
        // remaining heroes stay in reserve
    }
    return session;
}

} // namespace

// ---------------------------------------------------------------------------
// Pure formula
// ---------------------------------------------------------------------------

TEST_CASE("EnergyRules - base value with zero agility and zero bonuses is 1000") {
    REQUIRE(ComputeDailyStartingEnergy(0, 0, 0) == 1000);
}

TEST_CASE("EnergyRules - agility scales by 100 per point") {
    REQUIRE(ComputeDailyStartingEnergy(8, 0, 0) == 1800);
    REQUIRE(ComputeDailyStartingEnergy(5, 0, 0) == 1500);
}

TEST_CASE("EnergyRules - negative agility is floored to zero (never below base)") {
    REQUIRE(ComputeDailyStartingEnergy(-4, 0, 0) == 1000);
}

TEST_CASE("EnergyRules - leader passive and item bonus seams add through") {
    REQUIRE(ComputeDailyStartingEnergy(8, 50, 25) == 1875);
}

// ---------------------------------------------------------------------------
// GameSession daily reset
// ---------------------------------------------------------------------------

TEST_CASE("GameSession - ApplyDailyStartingEnergy sets current and max to the formula value") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.MaxEnergy() == 1800);
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("GameSession - daily energy uses the lowest agility across active and reserve") {
    // hero_a (agility 8) is active; hero_b (agility 3) stays in reserve.
    // The lowest across the entire traveling party is 3 -> 1000 + 300 = 1300.
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8), MakeHero("hero_b", 3) });
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.MaxEnergy() == 1300);
    REQUIRE(session.CurrentEnergy() == 1300);
}

TEST_CASE("GameSession - empty party yields the base 1000") {
    GameSession session;
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.MaxEnergy() == 1000);
    REQUIRE(session.CurrentEnergy() == 1000);
}

TEST_CASE("GameSession - party present but no unit catalog falls back to base 1000") {
    GameSession session;
    session.SetLeaderCapableUnitIds({ "hero_a" });
    REQUIRE(session.AddOwnedUnit("hero_a", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_a"));
    // No SetUnitCatalog: agility is unresolvable -> 1000 floor, never a crash.
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.MaxEnergy() == 1000);
    REQUIRE(session.CurrentEnergy() == 1000);
}

// ---------------------------------------------------------------------------
// Save / load
// ---------------------------------------------------------------------------

TEST_CASE("GameSession - energy round-trips through save/load") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.CurrentEnergy() == 1800);

    const core::SaveData data = session.ToSaveData();
    REQUIRE(data.energy == 1800);
    REQUIRE(data.maxEnergy == 1800);

    GameSession restored;
    restored.ApplySaveData(data);
    REQUIRE(restored.MaxEnergy() == 1800);
    REQUIRE(restored.CurrentEnergy() == 1800);
}

TEST_CASE("GameSession - legacy save without energy keys recomputes a non-zero pool") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();

    core::SaveData data = session.ToSaveData();
    // Simulate a pre-M14 save: the energy keys were absent (sentinel -1).
    data.energy = -1;
    data.maxEnergy = -1;

    GameSession restored;
    restored.ApplySaveData(data);
    // No unit catalog on the restored bare session -> 1000 floor, never 0.
    REQUIRE(restored.MaxEnergy() == 1000);
    REQUIRE(restored.CurrentEnergy() == 1000);
}

TEST_CASE("GameSession - saved current energy is clamped into [0, maxEnergy] on load") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();

    core::SaveData data = session.ToSaveData();
    data.maxEnergy = 1800;
    data.energy = 99999; // corrupt / out-of-range current

    GameSession restored;
    restored.ApplySaveData(data);
    REQUIRE(restored.MaxEnergy() == 1800);
    REQUIRE(restored.CurrentEnergy() == 1800); // clamped to max
}

// ---------------------------------------------------------------------------
// Independence from enemy-team energy
// ---------------------------------------------------------------------------

TEST_CASE("GameSession - team energy save key does not disturb enemy-team energy") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();

    EnemyTeamState red;
    red.teamColor = "Red";
    red.nodeId = "node_x";
    red.active = true;
    red.energy = 500;
    session.SetEnemyTeams({ red });

    const core::SaveData data = session.ToSaveData();
    REQUIRE(data.energy == 1800);            // player/team energy
    REQUIRE(data.enemyTeams.size() == 1);
    REQUIRE(data.enemyTeams[0].energy == 500); // enemy energy, distinct key

    // Restore onto a session that already has the Red team registered (enemy
    // restore matches by color into existing teams).
    GameSession restored;
    EnemyTeamState redShell;
    redShell.teamColor = "Red";
    restored.SetEnemyTeams({ redShell });
    restored.ApplySaveData(data);

    REQUIRE(restored.CurrentEnergy() == 1800);
    REQUIRE(restored.EnemyTeams().size() == 1);
    REQUIRE(restored.EnemyTeams()[0].energy == 500);
}
