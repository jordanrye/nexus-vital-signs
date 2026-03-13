#ifndef HEALING_ADDON_H
#define HEALING_ADDON_H

#include <string>

#include "layout_types.h"

namespace Addon {

void KeybindHandler(const char* aIdentifier, bool aIsRelease);
void Render();
void Options();

void ContentViewEmpty();
void ContentViewGeneral(std::string& name, std::string& colors, Position_t& position, Layout_t& layout);
void ContentViewIndicator(Indicator_t& indicator);

} // namespace Addon

#endif /* HEALING_ADDON_H */