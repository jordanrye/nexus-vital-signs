#ifndef JSON_ADDON_SERIALISERS
#define JSON_ADDON_SERIALISERS

#include "nlohmann/json.hpp"

#include "addon_types.h"
#include "layout_types.h"

using json = nlohmann::json;

void dser_Position_t(json& object, Position_t& position);
void dser_Layout_t(json& object, Layout_t& layout);
void dser_GeneralConfig_t(json& object, GeneralConfig_t& config);
void dser_Indicator_t(json& object, Indicator_t& indicator);
void dser_IconTextTrigger_t(json& object, IconTextTrigger_t& trigger);

json ser_Position_t(const Position_t& position);
json ser_Layout_t(const Layout_t& position);
json ser_GeneralConfig_t(const GeneralConfig_t& config);
json ser_Indicator_t(const Indicator_t& indicator);
json ser_IconTextTrigger_t(const IconTextTrigger_t& trigger);

#endif /* JSON_ADDON_SERIALISERS */