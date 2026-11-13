#ifndef VS_UI_COMMON_H
#define VS_UI_COMMON_H

#include "data_link/data_link.h"

#include "addon_types.h"

namespace UI {

    const int SQUAD_MEMBER_LIMIT = 50; // max number of player frames
    const int SQUAD_SUBGROUP_LIMIT = 15; // max number of subgroups
    const int BOON_LIMIT = 12; // max number of boons
    const int CONDITION_LIMIT = 14; // max number of conditions

    ImColor GetBackgroundColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::EHealthType healthType);
    ImColor GetHealthColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::EHealthType healthType, float health, VitalSignsDataLink::EProfession profession, VitalSignsDataLink::EPrimaryRole primaryRole);
    ImColor GetBarrierColour(const ColourPresets_t& config, const std::string& palette);

    Texture* GetOrCreateTexture(const std::string& textureSource, const std::string& texturePath);
    Texture* GetOrCreateTextureFromResource(const char* identifier, unsigned resourceId);

} // namespace UI

#endif /* VS_UI_COMMON_H */
