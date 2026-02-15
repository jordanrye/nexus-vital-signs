#include "json_imgui_serialisers.h"

void dser_ImVec4(const json& object, ImVec4 &vec)
{
    if (!object.is_null())
    {
        if (!object[0].is_null()) { object[0].get_to(vec.x); }
        if (!object[1].is_null()) { object[1].get_to(vec.y); }
        if (!object[2].is_null()) { object[2].get_to(vec.z); }
        if (!object[3].is_null()) { object[3].get_to(vec.w); }
    }
}

void dser_ImColor(const json& object, ImColor &color)
{
    if (!object.is_null())
    {
        if (!object[0].is_null()) { object[0].get_to(color.Value.x); } /* red */
        if (!object[1].is_null()) { object[1].get_to(color.Value.y); } /* green */
        if (!object[2].is_null()) { object[2].get_to(color.Value.z); } /* blue */
        if (!object[3].is_null()) { object[3].get_to(color.Value.w); } /* alpha */
    }
}

json ser_ImVec4(const ImVec4& vec)
{
    json object = json::array();
    object.push_back(vec.x);
    object.push_back(vec.y);
    object.push_back(vec.z);
    object.push_back(vec.w);
    return object;
}

json ser_ImColor(const ImColor& color)
{
    json object = json::array();
    object.push_back(color.Value.x); /* red */
    object.push_back(color.Value.y); /* green */
    object.push_back(color.Value.z); /* blue */
    object.push_back(color.Value.w); /* alpha */
    return object;
}