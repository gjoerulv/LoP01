#include "app/TradingPostInteraction.h"

#include <algorithm>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/economy/TradingPostTransactionRules.h"

namespace app {

namespace {

using gameplay::ResourceType;

// Decision 59: a Trading Post visit with at least one trade costs 20 minutes.
// Used when the service does not author its own visit cost.
constexpr int kDefaultVisitMinutes = 20;
constexpr int kMaxQuantity = 999;

const char* ModeLabel(int mode) {
    switch (mode) {
        case 0: return "Buy";
        case 1: return "Sell";
        case 2: return "Barter";
    }
    return "";
}

} // namespace

void TradingPostInteraction::Open(
    const gameplay::GameSession& session,
    const data::LocationServiceDefinition& service) {
    active_ = true;
    serviceId_ = service.id;
    visitTimeCostMinutes_ =
        service.timeCostMinutes > 0 ? service.timeCostMinutes : kDefaultVisitMinutes;
    mode_ = TradeMode::GoldBuy;
    selectedIndex_ = 0;
    quantity_ = 1;
    anyTradeSucceeded_ = false;
    lastResult_.clear();

    const auto offer = session.ResolveTradingPostOffer(serviceId_);
    effectiveTier_ = offer.effectiveTier;
    priceFactor_ = offer.priceFactor;
    barter_ = offer.barter;
}

int TradingPostInteraction::OptionCount() const {
    if (mode_ == TradeMode::Barter) {
        return static_cast<int>(barter_.size());
    }
    return static_cast<int>(gameplay::kNonGoldResourceCount);
}

TradingPostApplyResult TradingPostInteraction::ApplyCommand(
    const TradingPostCommand command,
    gameplay::GameSession& session) {
    TradingPostApplyResult result;
    if (!active_ || command == TradingPostCommand::None) {
        return result;
    }
    result.consumed = true;

    switch (command) {
        case TradingPostCommand::Exit: {
            result.shouldExit = true;
            if (anyTradeSucceeded_ && visitTimeCostMinutes_ > 0) {
                session.AddMinutes(visitTimeCostMinutes_);
                result.statusText = "Left Trading Post (" +
                    std::to_string(visitTimeCostMinutes_) + " min passed)";
            } else {
                result.statusText = "Left Trading Post";
            }
            return result;
        }
        case TradingPostCommand::CycleMode: {
            mode_ = static_cast<TradeMode>((static_cast<int>(mode_) + 1) % 3);
            selectedIndex_ = 0;
            quantity_ = 1;
            result.statusText = std::string("Mode: ") + ModeLabel(static_cast<int>(mode_));
            return result;
        }
        case TradingPostCommand::SelectPrev:
        case TradingPostCommand::SelectNext: {
            const int count = OptionCount();
            if (count <= 0) {
                return result;
            }
            const int delta = command == TradingPostCommand::SelectPrev ? -1 : 1;
            selectedIndex_ = std::clamp(selectedIndex_ + delta, 0, count - 1);
            return result;
        }
        case TradingPostCommand::QuantityDown:
        case TradingPostCommand::QuantityUp: {
            const int delta = command == TradingPostCommand::QuantityDown ? -1 : 1;
            quantity_ = std::clamp(quantity_ + delta, 1, kMaxQuantity);
            return result;
        }
        case TradingPostCommand::ConfirmTrade: {
            const int count = OptionCount();
            if (count <= 0) {
                lastResult_ = "Nothing to trade here";
                result.statusText = lastResult_;
                return result;
            }
            selectedIndex_ = std::clamp(selectedIndex_, 0, count - 1);

            gameplay::TradeResult trade;
            if (mode_ == TradeMode::Barter) {
                const auto& entry = barter_[selectedIndex_];
                trade = session.TryTradingPostBarter(serviceId_, entry.from, entry.to, quantity_);
            } else {
                const ResourceType resource = gameplay::kNonGoldResourceTypes[selectedIndex_];
                trade = mode_ == TradeMode::GoldBuy
                    ? session.TryTradingPostBuyForGold(serviceId_, resource, quantity_)
                    : session.TryTradingPostSellForGold(serviceId_, resource, quantity_);
            }

            if (trade.success) {
                anyTradeSucceeded_ = true;
            }
            lastResult_ = trade.message;
            result.statusText = lastResult_;
            return result;
        }
        case TradingPostCommand::None:
            return result;
    }
    return result;
}

void TradingPostInteraction::Close() {
    active_ = false;
    serviceId_.clear();
    visitTimeCostMinutes_ = 0;
    mode_ = TradeMode::GoldBuy;
    selectedIndex_ = 0;
    quantity_ = 1;
    anyTradeSucceeded_ = false;
    lastResult_.clear();
    barter_.clear();
    effectiveTier_ = 0;
    priceFactor_ = 100;
}

bool TradingPostInteraction::IsActive() const {
    return active_;
}

std::string TradingPostInteraction::BuildPromptText(const gameplay::GameSession& session) const {
    if (!active_) {
        return "Trading Post\nE: Open";
    }

    std::string out = "Trading Post (tier " + std::to_string(effectiveTier_) + ")\n";
    out += std::string("Mode: ") + ModeLabel(static_cast<int>(mode_)) + "\n";

    const int count = OptionCount();
    if (count <= 0) {
        out += "> (no trades offered)\n";
    } else {
        const int index = std::clamp(selectedIndex_, 0, count - 1);
        const std::string position =
            " (" + std::to_string(index + 1) + "/" + std::to_string(count) + ") [Left/Right]";

        if (mode_ == TradeMode::Barter) {
            const auto& entry = barter_[index];
            const auto quote = gameplay::economy::QuoteBarter(barter_, entry.from, entry.to, quantity_);
            out += "> Barter " + std::to_string(quote.fromCost) + " " +
                gameplay::ResourceTypeToString(entry.from) + " -> " + std::to_string(quantity_) +
                " " + gameplay::ResourceTypeToString(entry.to) + position + "\n";
        } else {
            const ResourceType resource = gameplay::kNonGoldResourceTypes[index];
            const auto quote = mode_ == TradeMode::GoldBuy
                ? gameplay::economy::QuoteBuyResourceForGold(resource, quantity_, priceFactor_)
                : gameplay::economy::QuoteSellResourceForGold(resource, quantity_, priceFactor_);
            const std::string verb = mode_ == TradeMode::GoldBuy ? "Buy " : "Sell ";
            const std::string arrow = mode_ == TradeMode::GoldBuy ? " <- " : " -> ";
            out += "> " + verb + std::to_string(quantity_) + " " +
                gameplay::ResourceTypeToString(resource) + arrow +
                std::to_string(quote.goldAmount) + " Gold" + position + "\n";
        }
    }

    out += "Qty: " + std::to_string(quantity_) + " [Up/Down]\n";
    out += "Gold: " + std::to_string(session.ResourceCount(ResourceType::Gold));
    for (const auto type : gameplay::kNonGoldResourceTypes) {
        const int held = session.ResourceCount(type);
        if (held > 0) {
            out += "  " + std::string(gameplay::ResourceTypeToString(type)) + ":" + std::to_string(held);
        }
    }
    out += "\n";
    if (!lastResult_.empty()) {
        out += lastResult_ + "\n";
    }
    out += "2 Mode  1 Trade  E Done";
    return out;
}

} // namespace app
