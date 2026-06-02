#include <catch2/catch_test_macros.hpp>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/economy/MinePayoutRules.h"

using data::LocationServiceKind;
using gameplay::economy::MineServiceIsPayable;

namespace {

// A fully-payable baseline: a mine with outputs, owned by "Green" (the receiving
// team), not locked/destroyed, not hostile-occupied.
bool Payable(LocationServiceKind kind = LocationServiceKind::Mine,
    bool hasOutputs = true,
    const std::string& owner = "Green",
    bool locked = false,
    bool destroyed = false,
    const std::string& receiver = "Green",
    bool hostile = false) {
    return MineServiceIsPayable(kind, hasOutputs, owner, locked, destroyed, receiver, hostile);
}

} // namespace

TEST_CASE("MinePayoutGate - owned mine with outputs pays") {
    REQUIRE(Payable());
}

TEST_CASE("MinePayoutGate - non-mine kind never pays") {
    REQUIRE_FALSE(Payable(LocationServiceKind::Shop));
    REQUIRE_FALSE(Payable(LocationServiceKind::Rest));
    REQUIRE_FALSE(Payable(LocationServiceKind::Recruit));
}

TEST_CASE("MinePayoutGate - mine without authored outputs does not pay") {
    REQUIRE_FALSE(Payable(LocationServiceKind::Mine, /*hasOutputs=*/false));
}

TEST_CASE("MinePayoutGate - unowned service does not pay") {
    REQUIRE_FALSE(Payable(LocationServiceKind::Mine, true, /*owner=*/""));
}

TEST_CASE("MinePayoutGate - service owned by another team does not pay the receiver") {
    // Allied/enemy/neutral ownership does not pay into the receiving team.
    REQUIRE_FALSE(Payable(LocationServiceKind::Mine, true, /*owner=*/"Red",
        false, false, /*receiver=*/"Green"));
}

TEST_CASE("MinePayoutGate - locked service does not pay") {
    REQUIRE_FALSE(Payable(LocationServiceKind::Mine, true, "Green", /*locked=*/true));
}

TEST_CASE("MinePayoutGate - destroyed service does not pay") {
    REQUIRE_FALSE(Payable(LocationServiceKind::Mine, true, "Green", false, /*destroyed=*/true));
}

TEST_CASE("MinePayoutGate - hostile-occupied service does not pay") {
    REQUIRE_FALSE(Payable(LocationServiceKind::Mine, true, "Green", false, false,
        "Green", /*hostile=*/true));
}
