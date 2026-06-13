#pragma once

#include <string>

#include "app/ShellSelectionRules.h"
#include "data/ContentRepository.h"
#include "rendering/CampaignSelectRenderer.h"
#include "rendering/TitleRenderer.h"

namespace app::mappers
{
    // Builds the shell screen models (main menu, game-mode select, campaign
    // select, standalone scenario select). Pure presentation assembly: all
    // playability verdicts come from app::shell::ShellSelectionRules and all
    // content comes from the loaded ContentRepository. Selection lists reuse the
    // generic CampaignSelectModel list shape.
    class ShellModelMapper
    {
    public:
        // Fixed main-menu item order. Continue is disabled without a usable
        // save; New Game is disabled when content could not be loaded; Quit is
        // always available.
        static constexpr int kMainMenuContinueIndex = 0;
        static constexpr int kMainMenuNewGameIndex = 1;
        static constexpr int kMainMenuQuitIndex = 2;
        static constexpr int kMainMenuItemCount = 3;

        // Fixed game-mode-select row order (Tutorial/PvP stay hidden until real
        // content/systems exist for them; docs/game_shell_flow.md §7/§9).
        static constexpr int kGameModeCampaignIndex = 0;
        static constexpr int kGameModeScenarioIndex = 1;
        static constexpr int kGameModeRowCount = 2;

        [[nodiscard]] ashvale::rendering::TitleScreenModel MapMainMenu(
            bool hasSave,
            bool contentLoaded,
            int selectedIndex,
            const std::string& statusText) const;

        [[nodiscard]] ashvale::rendering::CampaignSelectModel MapGameModeSelect(
            const data::ContentRepository& content,
            int selectedIndex,
            const std::string& statusText) const;

        [[nodiscard]] ashvale::rendering::CampaignSelectModel MapCampaignSelect(
            const data::ContentRepository& content,
            const shell::ShellEntryStatus& contentGate,
            int selectedIndex,
            const std::string& statusText) const;

        [[nodiscard]] ashvale::rendering::CampaignSelectModel MapScenarioSelect(
            const data::ContentRepository& content,
            const shell::ShellEntryStatus& contentGate,
            int selectedIndex,
            const std::string& statusText) const;
    };
}
