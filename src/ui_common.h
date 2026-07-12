#ifndef VS_UI_COMMON_H
#define VS_UI_COMMON_H

#include "data_link/data_link.h"

#include "addon_types.h"

namespace UI {

    const int SQUAD_MEMBER_LIMIT = 50; // max number of player frames
    const int BOON_LIMIT = 12; // max number of boons
    const int CONDITION_LIMIT = 14; // max number of conditions

    ImColor GetBackgroundColour(const ColourPresets_t& config, const std::string& palette);
    ImColor GetHealthColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::E_HEALTH_TYPE healthType, float health, VitalSignsDataLink::EProfession profession);
    ImColor GetBarrierColour(const ColourPresets_t& config, const std::string& palette);

    Texture* GetOrCreateTexture(const std::string& textureSource, const std::string& texturePath);

} // namespace UI

#endif /* VS_UI_COMMON_H */
