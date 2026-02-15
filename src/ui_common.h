#ifndef VS_UI_COMMON_H
#define VS_UI_COMMON_H

#include "interface/vital_signs_interface.h"

#include "addon_types.h"

namespace UI {

    const int SQUAD_MEMBER_LIMIT = 10; // max items in the radial menu
    const int BOON_LIMIT = 12; // max number of boons
    const int CONDITION_LIMIT = 14; // max number of conditions

    ImColor GetHealthColour(const ColourPresets_t& config, VitalSignsData::E_HEALTH_TYPE healthType);

    Texture* GetOrCreateTexture(const std::string& textureSource, const std::string& texturePath);

} // namespace UI

#endif /* VS_UI_COMMON_H */
