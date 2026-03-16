#pragma once

#include <string>

namespace core {

class GameClock {
public:
    static constexpr int kDayStartHour = 6;
    static constexpr int kDayEndHour = 2;
    static constexpr int kMinutesPerSliceDay = 20 * 60;

    GameClock();

    void AdvanceMinutes(int minutes);
    void SetToWakePenaltyStart();

    [[nodiscard]] int Day() const;
    [[nodiscard]] int MinutesIntoSliceDay() const;
    [[nodiscard]] std::string TimeString() const;
    [[nodiscard]] bool IsBeforeRegionTransferDeadline() const;

    static int QuantizeTravelMinutes(int rawMinutes);

private:
    int day_;
    int minutesIntoSliceDay_;
};

} // namespace core
