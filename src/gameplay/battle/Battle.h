#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace gameplay::battle {

enum class TeamSide {
    Allies,
    Enemies
};

enum class UnitCategory {
    Leader,
    Hero,
    Generic
};

enum class UnitRange {
    Melee = 0,
    Ranged = 1,
    LongRanged = 3
};

enum class UnitPosition {
    Front = 0,
    Middle = 1,
    Back = 2,
    Leader = 5
};

enum class BattleActionType {
    Attack,
    Defend,
    Wait,
    Skill1,
    Skill2
};

struct UnitStats {
    int attack = 0;
    int defense = 0;
    int magic = 0;
    int resistance = 0;
    int minDamage = 1;
    int maxDamage = 1;
    int maxHp = 1;
    int maxMp = 0;
    int agility = 1;
    int life = 1;
    UnitPosition position = UnitPosition::Front;
    UnitRange range = UnitRange::Melee;
};

struct BattleUnit {
    std::string id;
    std::string name;
    TeamSide side = TeamSide::Allies;
    UnitCategory category = UnitCategory::Generic;
    UnitStats stats;
    bool isPlayerCharacter = false;

    int hp = 1;
    int mp = 0;
    int life = 1;
    bool defending = false;
    bool ko = false;
    bool lostAfterBattle = false;
    bool inReserve = false;
    int timeToAct = 100;
};

struct BattleSummary {
    bool finished = false;
    bool alliesWon = false;
    bool enemiesWon = false;
    bool playerSetToOneHp = false;
};

class BattleState {
public:
    static constexpr int kMaxUnitsPerSide = 5;

    static BattleState CreateDebugBattle(uint32_t seed = 7);
    static BattleState CreateForTests(std::vector<BattleUnit> units, uint32_t seed = 7);

    explicit BattleState(uint32_t seed = 7);

    bool SetUnits(std::vector<BattleUnit> units);

    [[nodiscard]] bool IsFinished() const;
    [[nodiscard]] const BattleSummary& Summary() const;
    [[nodiscard]] int ActiveUnitIndex() const;
    [[nodiscard]] const std::vector<BattleUnit>& Units() const;
    [[nodiscard]] std::vector<int> UpcomingTurnOrder(int count) const;
    [[nodiscard]] std::string LastActionText() const;

    [[nodiscard]] int FindFirstTargetForActive() const;

    bool ExecuteAction(BattleActionType action, int targetIndex);

private:
    [[nodiscard]] bool IsAliveAndActive(const BattleUnit& unit) const;
    [[nodiscard]] bool IsActionTargetable(const BattleUnit& unit) const;
    [[nodiscard]] int FindLeaderIndex(TeamSide side) const;
    [[nodiscard]] int EffectiveStat(const BattleUnit& unit, TeamSide side, int UnitStats::*field) const;
    [[nodiscard]] int ComputePhysicalDamage(const BattleUnit& attacker, const BattleUnit& defender);
    [[nodiscard]] int ComputeMagicDamage(const BattleUnit& attacker, const BattleUnit& defender, float power);
    [[nodiscard]] int ApplyDamage(BattleUnit& target, int damage);
    [[nodiscard]] int PostActionDelay(const BattleUnit& attacker, const BattleUnit& defender, bool waited) const;
    [[nodiscard]] float AgilityPenalty(const BattleUnit& attacker, const BattleUnit& defender) const;

    void AdvanceToNextTurn();
    void ShiftPositions(TeamSide side);
    void FinalizeIfFinished();

    std::mt19937 random_;
    std::vector<BattleUnit> units_;
    int activeUnitIndex_ = -1;
    BattleSummary summary_;
    std::string lastActionText_;
};

} // namespace gameplay::battle
