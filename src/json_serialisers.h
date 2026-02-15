#ifndef JSON_SERIALISERS
#define JSON_SERIALISERS

#include "nlohmann/json.hpp"

using json = nlohmann::json;

template <typename T>
void dser_BasicType(json& object, T& data)
{
    if (!object.is_null())
    {
        object.get_to(data);
    }
}

#endif /* JSON_SERIALISERS */
