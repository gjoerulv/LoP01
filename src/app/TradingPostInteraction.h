#pragma once

#include <string>
#include <vector>

#include "gameplay/ResourceState.h"
#include "gameplay/economy/TraderOwnershipRules.h"  // ResolvedExchange

namespace gameplay {
class GameSession;
}

namespace data {
struct LocationServiceDefinition;
}

namespace app {

enum class TradingPostCommand {
    None,
    SelectPrev,    // previous option in the current mode's list
    SelectNext,    // next option
    QuantityDown,  // decrease trade quantity (min 1)
    QuantityUp,    // increase trade quantity
    CycleMode,     // GoldBuy -> GoldSell -> Barter -> GoldBuy
    ConfirmTrade,  // execute the selected trade
    Exit           // leave the visit (applies the per-visit time cost iff a trade succeeded)
};

struct TradingPostApplyResult {
    bool consumed = false;
    bool shouldExit = false;
    std::string statusText;
};

// In-place, multi-step Trading Post visit. Mirrors MusteringInteraction: the App
// owns one instance, opens it from the service dispatch, routes input commands to
// ApplyCommand while IsActive(), and renders BuildPromptText. Trades execute
// through the GameSession TryTradingPost* APIs (atomic, re-gated); this object
// holds only transient visit/selection state and never re-implements economy or
// gating logic. The per-visit time cost (Decision 59) is applied once on Exit and
// only when at least one trade succeeded during the visit.
class TradingPostInteraction {
public:
    void Open(const gameplay::GameSession& session, const data::LocationServiceDefinition& service);
    TradingPostApplyResult ApplyCommand(TradingPostCommand command, gameplay::GameSession& session);
    void Close();
    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] std::string BuildPromptText(const gameplay::GameSession& session) const;

private:
    enum class TradeMode { GoldBuy, GoldSell, Barter };

    [[nodiscard]] int OptionCount() const;

    bool active_ = false;
    std::string serviceId_;
    int visitTimeCostMinutes_ = 0;

    TradeMode mode_ = TradeMode::GoldBuy;
    int selectedIndex_ = 0;
    int quantity_ = 1;

    bool anyTradeSucceeded_ = false;
    std::string lastResult_;

    // Offer resolved once at Open. Trade rates are stable for the visit's
    // lifetime (ownership/occupation cannot change inside the modal), so caching
    // them avoids a per-frame catalog scan; live Gold/resource counts are read
    // from the session in BuildPromptText. `offerUsable_` is false for a locked,
    // destroyed, hostile-occupied, or non-Trading-Post service: no trades are
    // offered and confirms are refused (the transaction APIs would refuse anyway).
    bool offerUsable_ = false;
    int effectiveTier_ = 0;
    int priceFactor_ = 100;
    std::vector<gameplay::economy::ResolvedExchange> barter_;
};

} // namespace app
