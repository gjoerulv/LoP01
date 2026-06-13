#pragma once

namespace gameplay::battle {

// M33 threat-preview band. A bounded, player-facing danger estimate
// (docs/core_loop_rules.md §16) — deliberately NOT a battle simulation. `Unknown`
// is used when the conflict is not visible/known under the M32 reveal rules.
enum class ThreatBand { Unknown, Low, Even, Dangerous, Overwhelming };

// Cheap §16 estimate of the danger TO THE PLAYER given a force power for the
// player's side and the opposing side. The caller supplies battle-stat-derived
// powers (the existing UnitDefensePower/StackDefensePower proxy); this turns the
// ratio into a band using the §16 thresholds:
//   player share >= 0.80  -> Low          (player heavily favored)
//   player share <  0.25  -> Overwhelming (enemy heavily favored)
//   player share <  0.50  -> Dangerous    (enemy favored)
//   otherwise             -> Even
// Both powers <= 0 -> Even (no meaningful force on either side). Pure.
[[nodiscard]] ThreatBand EstimateThreatBand(long long playerPower, long long enemyPower);

// Short, stable, player-facing label for a band ("Low" / "Even" / "Dangerous" /
// "Overwhelming" / "" for Unknown). Not an id; safe for UI.
[[nodiscard]] const char* ThreatBandLabel(ThreatBand band);

} // namespace gameplay::battle
