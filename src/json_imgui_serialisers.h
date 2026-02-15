#ifndef JSON_IMGUI_SERIALISERS
#define JSON_IMGUI_SERIALISERS

#include <string>

#include "imgui/imgui.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void dser_ImVec4(const json& object, ImVec4& vec);
void dser_ImColor(const json& object, ImColor& color);

json ser_ImVec4(const ImVec4& vec);
json ser_ImColor(const ImColor& color);

#endif /* JSON_IMGUI_SERIALISERS */