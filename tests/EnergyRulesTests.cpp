#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "app/mappers/HudModelMapper.h"
#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/ContentRepository.h"
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

// A Hero (leader-capable) carrying one LeaderEnergy passive per listed amount.
data::UnitDefinition MakeLeaderWithEnergyBonuses(const std::string& id, int agility,
    const std::vector<int>& bonuses) {
    auto def = MakeHero(id, agility);
    for (const int amount : bonuses) {
        def.passiveEffects.push_back(data::UnitPassiveEffect{
            data::PassiveEffectKind::LeaderEnergy, "", "", amount});
    }
    return def;
}

// A Generic unit (never leader-capable) carrying a LeaderEnergy passive.
data::UnitDefinition MakeGenericWithEnergyBonus(const std::string& id, int agility, int amount) {
    data::UnitDefinition def;
    def.id = id;
    def.name = id;
    def.category = data::UnitDefinitionCategory::Generic;
    def.stats.agility = agility;
    def.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::LeaderEnergy, "", "", amount});
    return def;
}

// A Hero (leader-capable) carrying only a MineProduction passive — used to prove
// mine-production effects never leak into the Energy (Y) term.
data::UnitDefinition MakeLeaderWithMinePassive(const std::string& id, int agility, int amount) {
    auto def = MakeHero(id, agility);
    def.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, "Stone", "mine", amount});
    return def;
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
// Leader passive Energy bonus (the Y seam, fed from the spine)
// ---------------------------------------------------------------------------

TEST_CASE("GameSession - the leader's LeaderEnergy passive raises daily starting Energy") {
    auto session = MakeSessionWithParty({ MakeLeaderWithEnergyBonuses("hero_a", 8, {50}) });
    session.ApplyDailyStartingEnergy();
    // 1000 + 8*100 + 50
    REQUIRE(session.MaxEnergy() == 1850);
    REQUIRE(session.CurrentEnergy() == 1850);
}

TEST_CASE("GameSession - multiple LeaderEnergy passives on the leader sum") {
    auto session = MakeSessionWithParty({ MakeLeaderWithEnergyBonuses("hero_a", 8, {50, 25}) });
    session.ApplyDailyStartingEnergy();
    // 1000 + 800 + (50 + 25)
    REQUIRE(session.MaxEnergy() == 1875);
}

TEST_CASE("GameSession - a non-leader unit's LeaderEnergy passive does not count") {
    // hero_a (leader-capable, no passive) is active; gen_b (generic, +999) stays
    // in reserve and is never the leader -> its passive is ignored.
    auto session = MakeSessionWithParty({
        MakeHero("hero_a", 5),
        MakeGenericWithEnergyBonus("gen_b", 5, 999)
    });
    session.ApplyDailyStartingEnergy();
    // 1000 + min(5,5)*100, no +999
    REQUIRE(session.MaxEnergy() == 1500);
}

TEST_CASE("GameSession - a leader without a LeaderEnergy passive leaves Energy at the base formula") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.MaxEnergy() == 1800);  // 1000 + 800, Y = 0
}

TEST_CASE("GameSession - leader Energy bonus is applied at the day-boundary chokepoint") {
    auto session = MakeSessionWithParty({ MakeLeaderWithEnergyBonuses("hero_a", 8, {50}) });
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);  // cross one day
    REQUIRE(session.MaxEnergy() == 1850);
    REQUIRE(session.CurrentEnergy() == 1850);
}

TEST_CASE("GameSession - the leader's MineProduction passive does not affect daily Energy") {
    // Cross-consumer isolation: a MineProduction effect on the current leader
    // must never leak into the Energy (Y) term.
    auto session = MakeSessionWithParty({ MakeLeaderWithMinePassive("hero_a", 8, 50) });
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.MaxEnergy() == 1800);  // 1000 + 8*100, no MineProduction amount in Y
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

// ---------------------------------------------------------------------------
// M14-b — CanSpendEnergy
// ---------------------------------------------------------------------------

TEST_CASE("CanSpendEnergy - zero is always allowed") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.CanSpendEnergy(0));
}

TEST_CASE("CanSpendEnergy - negative is rejected") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();
    REQUIRE_FALSE(session.CanSpendEnergy(-1));
}

TEST_CASE("CanSpendEnergy - positive at or below current is allowed, above is rejected") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.CanSpendEnergy(1));
    REQUIRE(session.CanSpendEnergy(1800));       // exactly current
    REQUIRE_FALSE(session.CanSpendEnergy(1801)); // one over
}

// ---------------------------------------------------------------------------
// M14-b — TrySpendEnergy
// ---------------------------------------------------------------------------

TEST_CASE("TrySpendEnergy - zero succeeds and mutates nothing") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(0));
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("TrySpendEnergy - negative fails loudly and mutates nothing") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE_FALSE(session.TrySpendEnergy(-100));
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("TrySpendEnergy - insufficient fails and mutates nothing") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE_FALSE(session.TrySpendEnergy(1801));
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("TrySpendEnergy - sufficient subtracts and returns true") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(1000));
    REQUIRE(session.CurrentEnergy() == 800);
    REQUIRE(session.TrySpendEnergy(800)); // exact remainder
    REQUIRE(session.CurrentEnergy() == 0);
}

// ---------------------------------------------------------------------------
// M14-b — RecoverEnergy
// ---------------------------------------------------------------------------

TEST_CASE("RecoverEnergy - non-positive is a no-op") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy();
    REQUIRE(session.TrySpendEnergy(500)); // current 1300
    session.RecoverEnergy(0);
    REQUIRE(session.CurrentEnergy() == 1300);
    session.RecoverEnergy(-50);
    REQUIRE(session.CurrentEnergy() == 1300);
}

TEST_CASE("RecoverEnergy - positive recovers and clamps to the daily max") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // max 1800
    REQUIRE(session.TrySpendEnergy(1000)); // current 800
    session.RecoverEnergy(300);
    REQUIRE(session.CurrentEnergy() == 1100);
    session.RecoverEnergy(99999); // would overshoot
    REQUIRE(session.CurrentEnergy() == 1800); // clamped to max
}

// ---------------------------------------------------------------------------
// M14-b — day-boundary auto-reset
// ---------------------------------------------------------------------------

TEST_CASE("Energy auto-resets when AddMinutes crosses a day boundary") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(1500)); // current 300
    REQUIRE(session.CurrentEnergy() == 300);

    session.AddMinutes(core::GameClock::kMinutesPerSliceDay); // cross one day
    REQUIRE(session.CurrentEnergy() == 1800);
    REQUIRE(session.MaxEnergy() == 1800);
}

TEST_CASE("Energy does NOT reset for a sub-day AddMinutes") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(1500)); // current 300

    session.AddMinutes(60); // well within the same day
    REQUIRE(session.CurrentEnergy() == 300); // unchanged
}

TEST_CASE("Energy auto-resets via RestToNextDayStart") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(1500)); // current 300

    session.RestToNextDayStart();
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("Energy auto-resets via ApplyWakePenalty (wake next day with fresh Energy)") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(1500)); // current 300

    session.ApplyWakePenalty();
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("Energy resets exactly once across a multi-day AddMinutes jump") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(1500)); // current 300

    session.AddMinutes(core::GameClock::kMinutesPerSliceDay * 3); // jump 3 days
    // Reset is "set to formula value", not additive per day -> still 1800.
    REQUIRE(session.CurrentEnergy() == 1800);
    REQUIRE(session.MaxEnergy() == 1800);
}

// ---------------------------------------------------------------------------
// M14-c — SessionSnapshot exposure
// ---------------------------------------------------------------------------

TEST_CASE("SessionSnapshot reports current and max energy after a daily reset") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    const auto snap = session.Snapshot();
    REQUIRE(snap.energy == 1800);
    REQUIRE(snap.maxEnergy == 1800);
}

TEST_CASE("SessionSnapshot reflects a spend while max stays the daily ceiling") {
    auto session = MakeSessionWithParty({ MakeHero("hero_a", 8) });
    session.ApplyDailyStartingEnergy(); // 1800
    REQUIRE(session.TrySpendEnergy(500));
    const auto snap = session.Snapshot();
    REQUIRE(snap.energy == 1300);
    REQUIRE(snap.maxEnergy == 1800);
}

// ---------------------------------------------------------------------------
// M14-c — HudModelMapper exposure
// ---------------------------------------------------------------------------

TEST_CASE("HudModelMapper copies current and max energy from the snapshot") {
    // The mapper only reads content for area name lookups (falls back to ids
    // when absent), so a default-constructed repository is sufficient here.
    data::ContentRepository content;
    gameplay::GameSession session;

    auto snapshot = session.Snapshot();
    snapshot.energy = 1234;
    snapshot.maxEnergy = 1800;

    app::mappers::HudModelMapper mapper;
    const auto model = mapper.Map(content, session, snapshot, /*statusText=*/"", /*quests=*/{});

    REQUIRE(model.energy == 1234);
    REQUIRE(model.maxEnergy == 1800);
}
