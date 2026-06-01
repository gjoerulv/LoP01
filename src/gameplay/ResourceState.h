#pragma once

#include <array>
#include <cstddef>
#include <string>

namespace gameplay {

// M17 team resource model. The enum is strict: only these seven resources
// exist, matching docs/content_schema.md §7 and README_DECISIONS §52. Gold is a
// member so authored mine outputs and trader exchange data can name it, but Gold
// is NOT stored in the resource container — every Gold operation delegates to
// GameSession::gold_ (the single source of truth). The container physically
// stores only the six non-gold resources.
enum class ResourceType {
    Gold,
    Wood,
    Stone,
    Steel,
    Fiber,
    Clay,
    Gems
};

// Canonical authored/display names. Used for save serialization and the
// event-context resource bridge ("Wood" -> 10). Strict 1:1 with the enum.
[[nodiscard]] inline const char* ResourceTypeToString(ResourceType type) {
    switch (type) {
        case ResourceType::Gold:  return "Gold";
        case ResourceType::Wood:  return "Wood";
        case ResourceType::Stone: return "Stone";
        case ResourceType::Steel: return "Steel";
        case ResourceType::Fiber: return "Fiber";
        case ResourceType::Clay:  return "Clay";
        case ResourceType::Gems:  return "Gems";
    }
    return "";
}

// Strict parse. Returns false for any unknown string (no fuzzy matching, no
// case folding) so callers can reject invalid resource names explicitly.
[[nodiscard]] inline bool TryResourceTypeFromString(const std::string& value,
                                                    ResourceType& out) {
    if (value == "Gold")  { out = ResourceType::Gold;  return true; }
    if (value == "Wood")  { out = ResourceType::Wood;  return true; }
    if (value == "Stone") { out = ResourceType::Stone; return true; }
    if (value == "Steel") { out = ResourceType::Steel; return true; }
    if (value == "Fiber") { out = ResourceType::Fiber; return true; }
    if (value == "Clay")  { out = ResourceType::Clay;  return true; }
    if (value == "Gems")  { out = ResourceType::Gems;  return true; }
    return false;
}

[[nodiscard]] inline bool IsGoldResource(ResourceType type) {
    return type == ResourceType::Gold;
}

// The six non-gold resources, in canonical storage order. The resource
// container is a fixed array indexed by NonGoldResourceIndex(); iterating this
// array gives a stable, allocation-free order for persistence and bridging.
inline constexpr std::array<ResourceType, 6> kNonGoldResourceTypes = {
    ResourceType::Wood,
    ResourceType::Stone,
    ResourceType::Steel,
    ResourceType::Fiber,
    ResourceType::Clay,
    ResourceType::Gems
};

inline constexpr std::size_t kNonGoldResourceCount = kNonGoldResourceTypes.size();

// Dense index into the non-gold resource container. Gold has no index (it is
// never stored here); callers must guard with IsGoldResource() first.
[[nodiscard]] inline std::size_t NonGoldResourceIndex(ResourceType type) {
    switch (type) {
        case ResourceType::Wood:  return 0;
        case ResourceType::Stone: return 1;
        case ResourceType::Steel: return 2;
        case ResourceType::Fiber: return 3;
        case ResourceType::Clay:  return 4;
        case ResourceType::Gems:  return 5;
        case ResourceType::Gold:  return 0;  // unreachable: callers guard Gold
    }
    return 0;
}

} // namespace gameplay
