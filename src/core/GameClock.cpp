#include "core/GameClock.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace core {

GameClock::GameClock() : day_(1), minutesIntoSliceDay_(0) {}

void GameClock::AdvanceMinutes(int minutes) {
    if (minutes <= 0) {
        return;
    }

    minutesIntoSliceDay_ += minutes;
    while (minutesIntoSliceDay_ >= kMinutesPerSliceDay) {
        minutesIntoSliceDay_ -= kMinutesPerSliceDay;
        ++day_;
    }
}

void GameClock::SetToWakePenaltyStart() {
    minutesIntoSliceDay_ = (11 - kDayStartHour) * 60;
}

int GameClock::Day() const {
    return day_;
}

int GameClock::MinutesIntoSliceDay() const {
    return minutesIntoSliceDay_;
}

std::string GameClock::TimeString() const {
    const int absoluteMinutes = (kDayStartHour * 60 + minutesIntoSliceDay_) % (24 * 60);
    const int hour = absoluteMinutes / 60;
    const int minute = absoluteMinutes % 60;

    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0') << std::setw(2) << minute;
    return stream.str();
}

bool GameClock::IsBeforeRegionTransferDeadline() const {
    return minutesIntoSliceDay_ < (11 - kDayStartHour) * 60;
}

int GameClock::QuantizeTravelMinutes(const int rawMinutes) {
    const int clamped = std::clamp(rawMinutes, 15, 240);
    const int remainder = clamped % 15;
    if (remainder == 0) {
        return clamped;
    }

    return clamped + (15 - remainder);
}

} // namespace core
