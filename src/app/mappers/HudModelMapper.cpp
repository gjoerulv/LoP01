#include "app/mappers/HudModelMapper.h"

namespace app::mappers
{
    ashvale::rendering::HudModel HudModelMapper::Map(
        const gameplay::SessionSnapshot& snapshot,
        const std::string& statusText) const
    {
        ashvale::rendering::HudModel model;
        model.modeLabel = gameplay::GameSession::ToString(snapshot.mode);
        model.day = snapshot.day;
        model.timeText = snapshot.time;
        model.gold = snapshot.gold;
        model.primaryAreaLabel = "Region:";
        model.primaryAreaValue = snapshot.regionId;
        model.secondaryAreaLabel = "Location:";
        model.secondaryAreaValue = snapshot.destinationId;
        model.statusText = statusText;
        model.showStatus = true;
        return model;
    }
}