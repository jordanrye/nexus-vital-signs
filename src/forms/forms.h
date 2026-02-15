#pragma once

#include <string>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui_extensions.h"

#include "shared.h"

void form_Direction(std::string& direction);
void form_Font(std::string& fontSource, std::string& fontPath);
void form_FontPreset(std::string& fontSource, std::string& fontPath);
void form_FontSize(std::string& fontSource, float& fontSize);
void form_FontSizePreset(std::string& fontSource, float& fontSize);
void form_FontColour(std::string& source, ImColor& colour);
void form_FontDecorator(std::string& source, bool& isShadow, ImColor& colourShadow, bool& isOutline, ImColor& colourOutline);
void form_FontDecoratorPreset(bool& isShadow, ImColor& colourShadow, bool& isOutline, ImColor& colourOutline);
void form_ListLength(std::string& listLength);
void form_ListSpacing(int& listSpacing);
void form_Position(Position_t& position);
void form_SelectIndicatorType(std::string& indicatorType, bool isDisabled = false);
void form_SelectLayoutType(std::string& layoutType, bool isDisabled = false);
void form_SelectLayout(const std::vector<std::string>& layouts, std::string& selectedLayout);
void form_Size(Size_t& size);
void form_TextContent(std::string& textType, std::string& textCustom);
void form_TextLimit(std::string& limitType, float& limitValue);
void form_Texture(std::string& textureSource, std::string& texturePath);
void form_Trigger(Trigger_t& trigger);
