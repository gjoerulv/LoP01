#pragma once

#include <string>
#include <vector>

#include "data/ContentRepository.h"

namespace app::shell
{
    // Player-facing playability verdict for one shell selection entry. `reason`
    // is a short readable sentence; empty when the entry is playable. Raw
    // validation reports stay dev-facing (docs/game_shell_flow.md §12/§26).
    struct ShellEntryStatus
    {
        bool playable = true;
        std::string reason;
    };

    // Global content gate: nothing may start when content failed to load or the
    // installed content carries validation errors. This is the strict
    // do-not-silently-start-invalid-content gate; per-entry checks below refine
    // the message for individually broken entries.
    [[nodiscard]] ShellEntryStatus ContentGateStatus(
        bool contentLoaded, int validationErrorCount);

    // Per-entry reference checks. These are deliberately cheap (start ids must
    // resolve) — they exist so one broken authored entry can be shown disabled
    // with a reason while the rest of the list stays playable.
    [[nodiscard]] ShellEntryStatus ScenarioPlayability(
        const data::ContentRepository& content, const data::ScenarioDefinition& scenario);
    [[nodiscard]] ShellEntryStatus CampaignPlayability(
        const data::ContentRepository& content, const data::CampaignDefinition& campaign);

    // Scenarios eligible for the Standalone Scenario list. Only entries with
    // standaloneSelectable appear (docs/game_shell_flow.md §9/§11); campaign-only
    // scenarios are never listed.
    [[nodiscard]] std::vector<const data::ScenarioDefinition*> StandaloneScenarios(
        const data::ContentRepository& content);
}
