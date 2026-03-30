#pragma once

#include <string>
#include <vector>

namespace gameplay {
class GameSession;
}

namespace app {

enum class MusteringCommand {
    None,
    SelectReservePrev,
    SelectReserveNext,
    SelectActivePrev,
    SelectActiveNext,
    AddSelectedReserveToActive,
    RemoveSelectedActive,
    Exit
};

struct MusteringApplyResult {
    bool consumed = false;
    bool shouldExit = false;
    std::string statusText;
};

class MusteringInteraction {
public:
    void Open(const gameplay::GameSession& session);
    MusteringApplyResult ApplyCommand(MusteringCommand command, gameplay::GameSession& session);
    void Close();
    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] std::string BuildPromptText(const gameplay::GameSession& session) const;

private:
    struct ReserveCandidate {
        std::string unitId;
        int reserveCount = 0;
    };

    [[nodiscard]] std::vector<ReserveCandidate> BuildReserveCandidates(const gameplay::GameSession& session) const;
    void SanitizeSelectionIndices(const gameplay::GameSession& session);

    bool active_ = false;
    int selectedReserveIndex_ = -1;
    int selectedActiveIndex_ = -1;
};

} // namespace app
