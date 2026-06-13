#include "app/mappers/ShellModelMapper.h"

namespace app::mappers
{
    namespace
    {
        // Row playability = global content gate first, then the per-entry check,
        // so one broken entry reads its own reason while a broken install reads
        // the same global reason on every row.
        ashvale::rendering::CampaignSelectRow MakeRow(
            const std::string& id,
            const std::string& name,
            const std::string& description,
            const app::shell::ShellEntryStatus& contentGate,
            const app::shell::ShellEntryStatus& entryStatus,
            const bool selected)
        {
            ashvale::rendering::CampaignSelectRow row;
            row.id = id;
            row.name = name.empty() ? id : name;
            row.description = description;
            row.selected = selected;
            if (!contentGate.playable)
            {
                row.enabled = false;
                row.statusText = contentGate.reason;
            }
            else if (!entryStatus.playable)
            {
                row.enabled = false;
                row.statusText = "Unavailable: " + entryStatus.reason;
            }
            return row;
        }
    }

    ashvale::rendering::TitleScreenModel ShellModelMapper::MapMainMenu(
        const bool hasSave,
        const bool contentLoaded,
        const int selectedIndex,
        const std::string& statusText) const
    {
        ashvale::rendering::TitleScreenModel model;
        model.title = "Project Ashvale";
        model.subtitle = "Prototype Vertical Slice";
        model.menuItems = {"Continue", "New Game", "Quit"};
        model.menuItemEnabled = {hasSave, contentLoaded, true};
        model.selectedIndex = selectedIndex;
        model.statusText = statusText;
        model.footerHint = "Up/Down to choose, Enter to confirm";
        return model;
    }

    ashvale::rendering::CampaignSelectModel ShellModelMapper::MapGameModeSelect(
        const data::ContentRepository& content,
        const int selectedIndex,
        const std::string& statusText) const
    {
        ashvale::rendering::CampaignSelectModel model;
        model.title = "New Game";
        model.statusText = statusText;
        model.footerHint = "Up/Down to choose, Enter to confirm, Esc to go back";

        const bool hasCampaigns = !content.Campaigns().empty();
        const bool hasStandalone = !shell::StandaloneScenarios(content).empty();

        ashvale::rendering::CampaignSelectRow campaignRow;
        campaignRow.id = "campaign";
        campaignRow.name = "Campaign";
        campaignRow.description = "Play an authored campaign of linked scenarios.";
        campaignRow.selected = selectedIndex == kGameModeCampaignIndex;
        campaignRow.enabled = hasCampaigns;
        if (!hasCampaigns)
        {
            campaignRow.statusText = "Unavailable: no campaigns installed.";
        }
        model.campaigns.push_back(std::move(campaignRow));

        ashvale::rendering::CampaignSelectRow scenarioRow;
        scenarioRow.id = "standalone_scenario";
        scenarioRow.name = "Standalone Scenario";
        scenarioRow.description = "Play a single scenario.";
        scenarioRow.selected = selectedIndex == kGameModeScenarioIndex;
        scenarioRow.enabled = hasStandalone;
        if (!hasStandalone)
        {
            scenarioRow.statusText = "Unavailable: no standalone scenarios installed.";
        }
        model.campaigns.push_back(std::move(scenarioRow));

        return model;
    }

    ashvale::rendering::CampaignSelectModel ShellModelMapper::MapCampaignSelect(
        const data::ContentRepository& content,
        const shell::ShellEntryStatus& contentGate,
        const int selectedIndex,
        const std::string& statusText) const
    {
        ashvale::rendering::CampaignSelectModel model;
        model.title = "Select Campaign";
        model.statusText = statusText;
        model.emptyText = "No campaigns installed.";
        model.footerHint = "Up/Down to choose, Enter to start, Esc to go back";

        const auto& campaigns = content.Campaigns();
        for (int i = 0; i < static_cast<int>(campaigns.size()); ++i)
        {
            const auto& campaign = campaigns[i];
            model.campaigns.push_back(MakeRow(
                campaign.id, campaign.name, campaign.description,
                contentGate, shell::CampaignPlayability(content, campaign),
                i == selectedIndex));
        }
        return model;
    }

    ashvale::rendering::CampaignSelectModel ShellModelMapper::MapScenarioSelect(
        const data::ContentRepository& content,
        const shell::ShellEntryStatus& contentGate,
        const int selectedIndex,
        const std::string& statusText) const
    {
        ashvale::rendering::CampaignSelectModel model;
        model.title = "Select Scenario";
        model.statusText = statusText;
        model.emptyText = "No standalone scenarios installed.";
        model.footerHint = "Up/Down to choose, Enter to start, Esc to go back";

        const auto standalone = shell::StandaloneScenarios(content);
        for (int i = 0; i < static_cast<int>(standalone.size()); ++i)
        {
            const auto* scenario = standalone[i];
            // Scenarios author no description yet; the starting region's display
            // name is the most useful player-facing context available.
            std::string description;
            if (const auto* region = content.FindRegionById(scenario->startRegionId))
            {
                description = "Starts in " + region->name + ".";
            }
            model.campaigns.push_back(MakeRow(
                scenario->id, scenario->name, description,
                contentGate, shell::ScenarioPlayability(content, *scenario),
                i == selectedIndex));
        }
        return model;
    }
}
