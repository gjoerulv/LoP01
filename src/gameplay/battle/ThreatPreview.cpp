#include "gameplay/battle/ThreatPreview.h"

#include <algorithm>

namespace gameplay::battle {

ThreatBand EstimateThreatBand(long long playerPower, long long enemyPower) {
    playerPower = std::max<long long>(0, playerPower);
    enemyPower = std::max<long long>(0, enemyPower);
    const long long total = playerPower + enemyPower;
    if (total <= 0) {
        return ThreatBand::Even;
    }
    const double share = static_cast<double>(playerPower) / static_cast<double>(total);
    if (share >= 0.80) {
        return ThreatBand::Low;
    }
    if (share < 0.25) {
        return ThreatBand::Overwhelming;
    }
    if (share < 0.50) {
        return ThreatBand::Dangerous;
    }
    return ThreatBand::Even;
}

const char* ThreatBandLabel(const ThreatBand band) {
    switch (band) {
    case ThreatBand::Low:          return "Low";
    case ThreatBand::Even:         return "Even";
    case ThreatBand::Dangerous:    return "Dangerous";
    case ThreatBand::Overwhelming: return "Overwhelming";
    case ThreatBand::Unknown:      return "";
    }
    return "";
}

} // namespace gameplay::battle
