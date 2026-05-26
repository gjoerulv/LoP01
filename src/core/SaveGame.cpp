#include "core/SaveGame.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace core {

using nlohmann::json;

namespace {

constexpr size_t kCanonicalActiveSlotsV2 = 3;
constexpr size_t kCanonicalActiveSlotsV3 = 5;
constexpr size_t kCanonicalReserveSlots = 8;

int ParseStackIdSuffix(const std::string& stackId) {
    constexpr const char* kPrefix = "stk_";
    constexpr size_t kPrefixSize = 4;
    if (stackId.size() <= kPrefixSize || stackId.rfind(kPrefix, 0) != 0) {
        return -1;
    }

    const std::string suffix = stackId.substr(kPrefixSize);
    if (suffix.empty()) {
        return -1;
    }

    for (const char ch : suffix) {
        if (ch < '0' || ch > '9') {
            return -1;
        }
    }

    try {
        return std::stoi(suffix);
    }
    catch (...) {
        return -1;
    }
}

int ComputeNextStackIdCounter(const std::vector<RosterStackSaveState>& stacks) {
    int maxSuffix = 0;
    for (const auto& stack : stacks) {
        maxSuffix = std::max(maxSuffix, ParseStackIdSuffix(stack.stackId));
    }

    return std::max(1, maxSuffix + 1);
}

bool TryReadOptionalStringArray(const json& j, const char* key, std::vector<std::string>& output) {
    output.clear();
    if (!j.contains(key)) {
        return true;
    }
    if (!j[key].is_array()) {
        return false;
    }

    for (const auto& value : j[key]) {
        if (!value.is_string()) {
            return false;
        }
        output.push_back(value.get<std::string>());
    }

    return true;
}

void BuildCanonicalRosterFromLegacyFields(const json& j, SaveData& data) {
    std::map<std::string, int> normalizedOwned;
    if (j.contains("owned_unit_counts")) {
        if (!j["owned_unit_counts"].is_array()) {
            throw std::runtime_error("Invalid owned_unit_counts");
        }

        for (const auto& entryJson : j["owned_unit_counts"]) {
            if (!entryJson.is_object()) {
                throw std::runtime_error("Invalid owned_unit_counts entry");
            }
            if (!entryJson.contains("unit_id") || !entryJson["unit_id"].is_string()) {
                throw std::runtime_error("Invalid owned_unit_counts unit_id");
            }
            if (!entryJson.contains("count") ||
                !(entryJson["count"].is_number_integer() || entryJson["count"].is_number_unsigned())) {
                throw std::runtime_error("Invalid owned_unit_counts count");
            }

            const std::string unitId = entryJson["unit_id"].get<std::string>();
            const int count = entryJson["count"].get<int>();
            if (unitId.empty() || count <= 0) {
                continue;
            }

            normalizedOwned[unitId] += count;
            data.ownedUnitCounts.push_back(OwnedUnitCountSaveState{unitId, count});
        }
    }

    if (!TryReadOptionalStringArray(j, "active_party_unit_ids", data.activePartyUnitIds)) {
        throw std::runtime_error("Invalid active_party_unit_ids");
    }

    data.rosterStacks.clear();
    data.activeSlotStackIds.assign(kCanonicalActiveSlotsV3, "");
    data.reserveSlotStackIds.assign(kCanonicalReserveSlots, "");

    int nextSuffix = 1;
    auto allocateStackId = [&]() {
        return std::string("stk_") + std::to_string(nextSuffix++);
    };

    int activeWriteIndex = 0;
    for (const auto& unitId : data.activePartyUnitIds) {
        if (activeWriteIndex >= static_cast<int>(kCanonicalActiveSlotsV3) || unitId.empty()) {
            continue;
        }

        auto ownedIt = normalizedOwned.find(unitId);
        if (ownedIt == normalizedOwned.end() || ownedIt->second <= 0) {
            continue;
        }

        const std::string stackId = allocateStackId();
        data.rosterStacks.push_back(RosterStackSaveState{stackId, unitId, 1});
        data.activeSlotStackIds[activeWriteIndex] = stackId;
        ++activeWriteIndex;
        --ownedIt->second;
    }

    int reserveWriteIndex = 0;
    for (const auto& [unitId, count] : normalizedOwned) {
        if (count <= 0) {
            continue;
        }

        if (reserveWriteIndex >= static_cast<int>(kCanonicalReserveSlots)) {
            throw std::runtime_error("Legacy roster overflow");
        }

        const std::string stackId = allocateStackId();
        data.rosterStacks.push_back(RosterStackSaveState{stackId, unitId, count});
        data.reserveSlotStackIds[reserveWriteIndex] = stackId;
        ++reserveWriteIndex;
    }

    data.nextStackIdCounter = std::max(1, nextSuffix);
    data.hasCanonicalRoster = true;
}

void ParseCanonicalRosterFromJson(const json& j, SaveData& data) {
    if (!j["roster_stacks"].is_array() ||
        !j["active_slot_stack_ids"].is_array() ||
        !j["reserve_slot_stack_ids"].is_array()) {
        throw std::runtime_error("Invalid canonical roster structure");
    }

    data.rosterStacks.clear();
    for (const auto& entryJson : j["roster_stacks"]) {
        if (!entryJson.is_object() ||
            !entryJson.contains("stack_id") || !entryJson["stack_id"].is_string() ||
            !entryJson.contains("unit_id") || !entryJson["unit_id"].is_string() ||
            !entryJson.contains("quantity") ||
            !(entryJson["quantity"].is_number_integer() || entryJson["quantity"].is_number_unsigned())) {
            throw std::runtime_error("Invalid roster_stacks entry");
        }

        RosterStackSaveState stack;
        stack.stackId = entryJson["stack_id"].get<std::string>();
        stack.unitId = entryJson["unit_id"].get<std::string>();
        stack.quantity = entryJson["quantity"].get<int>();
        if (stack.stackId.empty() || stack.unitId.empty() || stack.quantity <= 0) {
            throw std::runtime_error("Invalid canonical stack value");
        }

        data.rosterStacks.push_back(std::move(stack));
    }

    data.activeSlotStackIds.clear();
    data.reserveSlotStackIds.clear();
    if (!TryReadOptionalStringArray(j, "active_slot_stack_ids", data.activeSlotStackIds) ||
        !TryReadOptionalStringArray(j, "reserve_slot_stack_ids", data.reserveSlotStackIds)) {
        throw std::runtime_error("Invalid canonical slot ids");
    }

    if (data.reserveSlotStackIds.size() != kCanonicalReserveSlots) {
        throw std::runtime_error("Invalid canonical slot counts");
    }

    if (data.schemaVersion == 5 || data.schemaVersion == 4 || data.schemaVersion == 3) {
        if (data.activeSlotStackIds.size() != kCanonicalActiveSlotsV3) {
            throw std::runtime_error("Invalid canonical slot counts");
        }
    } else if (data.schemaVersion == 2) {
        if (data.activeSlotStackIds.size() == kCanonicalActiveSlotsV2) {
            data.activeSlotStackIds.resize(kCanonicalActiveSlotsV3, "");
        } else {
            throw std::runtime_error("Invalid canonical slot counts");
        }
    } else {
        throw std::runtime_error("Invalid canonical slot counts");
    }

    std::set<std::string> stackIds;
    for (const auto& stack : data.rosterStacks) {
        if (!stackIds.insert(stack.stackId).second) {
            throw std::runtime_error("Duplicate stack id in canonical roster");
        }
    }

    std::set<std::string> assignedSlots;
    auto validateSlots = [&](const std::vector<std::string>& slots) {
        for (const auto& stackId : slots) {
            if (stackId.empty()) {
                continue;
            }

            if (stackIds.find(stackId) == stackIds.end()) {
                throw std::runtime_error("Slot references unknown stack id");
            }

            if (!assignedSlots.insert(stackId).second) {
                throw std::runtime_error("Duplicate slot stack id reference");
            }
        }
    };

    validateSlots(data.activeSlotStackIds);
    validateSlots(data.reserveSlotStackIds);

    if (j.contains("next_stack_id_counter") &&
        (j["next_stack_id_counter"].is_number_integer() || j["next_stack_id_counter"].is_number_unsigned())) {
        const int candidate = j["next_stack_id_counter"].get<int>();
        data.nextStackIdCounter = candidate >= 1 ? candidate : ComputeNextStackIdCounter(data.rosterStacks);
    }
    else {
        data.nextStackIdCounter = ComputeNextStackIdCounter(data.rosterStacks);
    }

    data.hasCanonicalRoster = true;
}

} // namespace

void to_json(json& j, const RecruitServiceState& data) {
    j = json{
        {"service_id", data.serviceId},
        {"remaining_stock", data.remainingStock},
        {"last_refresh_week", data.lastRefreshWeek}
    };
}

void to_json(json& j, const RosterStackSaveState& data) {
    j = json{
        {"stack_id", data.stackId},
        {"unit_id", data.unitId},
        {"quantity", data.quantity}
    };
}

void to_json(json& j, const OwnedUnitCountSaveState& data) {
    j = json{
        {"unit_id", data.unitId},
        {"count", data.count}
    };
}

void to_json(json& j, const DailyServiceState& data) {
    j = json{
        {"service_id", data.serviceId},
        {"remaining_uses_today", data.remainingUsesToday},
        {"last_refresh_day", data.lastRefreshDay}
    };
}

void from_json(const json& j, OwnedUnitCountSaveState& data) {
    j.at("unit_id").get_to(data.unitId);
    j.at("count").get_to(data.count);
}

void from_json(const json& j, RecruitServiceState& data) {
    j.at("service_id").get_to(data.serviceId);
    j.at("remaining_stock").get_to(data.remainingStock);
    j.at("last_refresh_week").get_to(data.lastRefreshWeek);
}

void from_json(const json& j, DailyServiceState& data) {
    j.at("service_id").get_to(data.serviceId);
    j.at("remaining_uses_today").get_to(data.remainingUsesToday);
    j.at("last_refresh_day").get_to(data.lastRefreshDay);
}

void to_json(json& j, const ItemSaveState& data) {
    j = json{
        {"item_id", data.itemId},
        {"quantity", data.quantity}
    };
}

void from_json(const json& j, ItemSaveState& data) {
    j.at("item_id").get_to(data.itemId);
    j.at("quantity").get_to(data.quantity);
}

void to_json(json& j, const ArtifactSaveState& data) {
    j = json{
        {"artifact_id", data.artifactId},
        {"quantity", data.quantity}
    };
}

void from_json(const json& j, ArtifactSaveState& data) {
    j.at("artifact_id").get_to(data.artifactId);
    j.at("quantity").get_to(data.quantity);
}

void to_json(json& j, const HeroEquipmentSaveState& data) {
    j = json{
        {"hero_id", data.heroId},
        {"attack_artifact_id",  data.attackArtifactId},
        {"defense_artifact_id", data.defenseArtifactId},
        {"misc1_artifact_id",   data.misc1ArtifactId},
        {"misc2_artifact_id",   data.misc2ArtifactId},
        {"misc3_artifact_id",   data.misc3ArtifactId}
    };
}

void from_json(const json& j, HeroEquipmentSaveState& data) {
    j.at("hero_id").get_to(data.heroId);
    data.attackArtifactId  = j.value("attack_artifact_id",  std::string{});
    data.defenseArtifactId = j.value("defense_artifact_id", std::string{});
    data.misc1ArtifactId   = j.value("misc1_artifact_id",   std::string{});
    data.misc2ArtifactId   = j.value("misc2_artifact_id",   std::string{});
    data.misc3ArtifactId   = j.value("misc3_artifact_id",   std::string{});
}

void to_json(json& j, const EnemyTeamSaveState& data) {
    j = json{
        {"team_color", data.teamColor},
        {"node_id", data.nodeId},
        {"active", data.active},
        {"energy", data.energy},
        {"cooldown_expires_at_minutes", data.cooldownExpiresAtMinutes},
        {"alliances", data.alliances}
    };
}

void from_json(const json& j, EnemyTeamSaveState& data) {
    j.at("team_color").get_to(data.teamColor);
    j.at("node_id").get_to(data.nodeId);
    j.at("active").get_to(data.active);
    data.energy = j.value("energy", 0);
    data.cooldownExpiresAtMinutes = j.value("cooldown_expires_at_minutes", 0);
    data.alliances.clear();
    if (j.contains("alliances") && j["alliances"].is_array()) {
        data.alliances = j["alliances"].get<std::vector<std::string>>();
    }
}

void to_json(json& j, const SaveData& data) {
    j = json{
        {"schema_version", data.schemaVersion},
        {"day", data.day},
        {"minutes_into_slice_day", data.minutesIntoSliceDay},
        {"gold", data.gold},
        {"mode", data.mode},
        {"region_id", data.regionId},
        {"destination_id", data.destinationId},
        {"completed_quest_ids", data.completedQuestIds},
        {"cleared_combat_node_ids", data.clearedCombatNodeIds},
        {"recruit_service_states", data.recruitServiceStates},
        {"daily_service_states", data.dailyServiceStates},
        {"travel_prep_discount_minutes", data.travelPrepDiscountMinutes},
        {"travel_prep_remaining_charges", data.travelPrepRemainingCharges},
        {"travel_prep_granted_day", data.travelPrepGrantedDay},
        {"roster_stacks", data.rosterStacks},
        {"active_slot_stack_ids", data.activeSlotStackIds},
        {"reserve_slot_stack_ids", data.reserveSlotStackIds},
        {"next_stack_id_counter", data.nextStackIdCounter},
        {"fired_event_ids", data.firedEventIds},
        {"story_flags", data.storyFlags},
        {"enemy_teams", data.enemyTeams},
        {"scenario_outcome_state", data.scenarioOutcomeState},
        {"scenario_outcome_matched_condition_index", data.scenarioOutcomeMatchedConditionIndex},
        {"scenario_outcome_reason", data.scenarioOutcomeReason},
        {"items", data.items},
        {"artifacts", data.artifacts},
        {"hero_equipment", data.heroEquipment}
    };
}

void from_json(const json& j, SaveData& data) {
    data = SaveData{};
    data.schemaVersion = j.value("schema_version", 1);

    j.at("day").get_to(data.day);
    j.at("minutes_into_slice_day").get_to(data.minutesIntoSliceDay);
    j.at("gold").get_to(data.gold);
    j.at("mode").get_to(data.mode);
    j.at("region_id").get_to(data.regionId);
    j.at("destination_id").get_to(data.destinationId);

    data.completedQuestIds.clear();
    if (j.contains("completed_quest_ids") && j["completed_quest_ids"].is_array()) {
        data.completedQuestIds = j["completed_quest_ids"].get<std::vector<std::string>>();
    }

    data.clearedCombatNodeIds.clear();
    if (j.contains("cleared_combat_node_ids") && j["cleared_combat_node_ids"].is_array()) {
        data.clearedCombatNodeIds = j["cleared_combat_node_ids"].get<std::vector<std::string>>();
    }
    data.recruitServiceStates.clear();
    if (j.contains("recruit_service_states") && j["recruit_service_states"].is_array()) {
        data.recruitServiceStates = j["recruit_service_states"].get<std::vector<RecruitServiceState>>();
    }

    data.dailyServiceStates.clear();
    if (j.contains("daily_service_states") && j["daily_service_states"].is_array()) {
        data.dailyServiceStates = j["daily_service_states"].get<std::vector<DailyServiceState>>();
    }

    data.travelPrepDiscountMinutes = j.value("travel_prep_discount_minutes", 0);
    data.travelPrepRemainingCharges = j.value("travel_prep_remaining_charges", 0);
    data.travelPrepGrantedDay = j.value("travel_prep_granted_day", 0);

    if (!TryReadOptionalStringArray(j, "fired_event_ids", data.firedEventIds)) {
        throw std::runtime_error("Invalid fired_event_ids");
    }
    if (!TryReadOptionalStringArray(j, "story_flags", data.storyFlags)) {
        throw std::runtime_error("Invalid story_flags");
    }

    data.enemyTeams.clear();
    if (j.contains("enemy_teams") && j["enemy_teams"].is_array()) {
        data.enemyTeams = j["enemy_teams"].get<std::vector<EnemyTeamSaveState>>();
    }

    data.scenarioOutcomeState = j.value("scenario_outcome_state", std::string{});
    data.scenarioOutcomeMatchedConditionIndex = j.value("scenario_outcome_matched_condition_index", -1);
    data.scenarioOutcomeReason = j.value("scenario_outcome_reason", std::string{});

    // M13-b inventory + equipment. Missing keys load as empty (legacy saves).
    data.items.clear();
    if (j.contains("items") && j["items"].is_array()) {
        data.items = j["items"].get<std::vector<ItemSaveState>>();
    }
    data.artifacts.clear();
    if (j.contains("artifacts") && j["artifacts"].is_array()) {
        data.artifacts = j["artifacts"].get<std::vector<ArtifactSaveState>>();
    }
    data.heroEquipment.clear();
    if (j.contains("hero_equipment") && j["hero_equipment"].is_array()) {
        data.heroEquipment = j["hero_equipment"].get<std::vector<HeroEquipmentSaveState>>();
    }

    const bool hasCanonicalStructuralFields =
        j.contains("roster_stacks") &&
        j.contains("active_slot_stack_ids") &&
        j.contains("reserve_slot_stack_ids");

    if (hasCanonicalStructuralFields) {
        ParseCanonicalRosterFromJson(j, data);
        return;
    }

    BuildCanonicalRosterFromLegacyFields(j, data);
}

bool SaveGameRepository::SaveToFile(const SaveData& saveData, const std::string& filePath) const {
    std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream output(path, std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output << json(saveData).dump(2);
    return true;
}

std::optional<SaveData> SaveGameRepository::LoadFromFile(const std::string& filePath) const {
    std::ifstream input(filePath);
    if (!input.is_open()) {
        return std::nullopt;
    }

    try {
        json j;
        input >> j;
        return j.get<SaveData>();
    }
    catch (...) {
        return std::nullopt;
    }
}

} // namespace core
