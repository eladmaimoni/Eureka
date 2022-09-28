#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>
#include "json_io.hpp"
#include "json_nlohmann_specializations.hpp"

//
// include all data types to be serialized
//
#include "AppTypes.hpp"

//////////////////////////////////////////////////////////////////////////
//
//                   IMPLEMENT_JSON_SERIALIZATION
//
//////////////////////////////////////////////////////////////////////////
#define IMPLEMENT_JSON_SERIALIZATION(type_name, ...)                      \
void to_json(nlohmann::json& nlohmann_json_j, const type_name& nlohmann_json_t)   \
{                                                                         \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))      \
}                                                                                 \
void from_json(const nlohmann::json& nlohmann_json_j, type_name& nlohmann_json_t) \
{                                                                                 \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, __VA_ARGS__))    \
}                                                                                 \
template <>                                                                       \
void to_json_file(const type_name& t, const std::filesystem::path& f)               \
{                                                                                 \
    nlohmann::json j;                                                             \
    to_json(j, t);                                                                \
                                                                                  \
    {                                                                             \
        std::ofstream file(f);                                                    \
        file << j.dump(4, ' ');                                                   \
    }                                                                             \
}                                                                                 \
                                                                                  \
template <>                                                                       \
type_name from_json_file(const std::filesystem::path& f)                          \
{                                                                                 \
    if (!std::filesystem::exists(f))                                              \
    {                                                                             \
        throw std::invalid_argument(f.string() + " file not found");              \
    }                                                                             \
    std::ifstream ifs(f);                                                         \
    std::stringstream buffer;                                                     \
    buffer << ifs.rdbuf();                                                        \
    auto j = nlohmann::json::parse(buffer);                                       \
                                                                                  \
    type_name t;                                                                  \
    from_json(j, t);                                                              \
    return t;                                                                     \
}                                                                                 

#define JSON_GET(obj, field_name, jsn, type)                                           \
    if (!j.contains(#field_name))                                                      \
    {                                                                                  \
        throw std::invalid_argument((std::string(#field_name) + " not found").c_str());\
    }                                                                                  \
    else                                                                               \
    {                                                                                  \
        obj.field_name = jsn[#field_name].get<type>();                                 \
    }

#define JSON_GET_LENIENT(obj, field_name, jsn, type, default_val)                      \
    if (!j.contains(#field_name))                                                      \
    {                                                                                  \
        obj.field_name = default_val;                                                  \
    }                                                                                  \
    else                                                                               \
    {                                                                                  \
        obj.field_name = jsn[#field_name].get<type>();                                 \
    }
#define FROM_JSON_LENIENT(obj, field_name, jsn) \
if (jsn.contains(#field_name))                  \
{                                               \
    from_json(jsn[#field_name], obj.field_name);\
}
#define JSON_SET(obj, field_name, jsn) jsn[#field_name] = obj.field_name



namespace eureka
{
    NLOHMANN_JSON_SERIALIZE_ENUM(StamEnum,
        {
            {StamEnum::eOne, "eOne"},
            {StamEnum::eTwo, "eTwo"}
        });

    IMPLEMENT_JSON_SERIALIZATION(WindowPosition,
        x,
        y
    );

    IMPLEMENT_JSON_SERIALIZATION(WindowConfig,
        position,
        width,
        height
    );

    IMPLEMENT_JSON_SERIALIZATION(LiveSlamUIMemo,
        previously_used_ips,
        show_filter_constraints,
        show_pnp_inliers,
        show_pnp_outliers,
        show_gpo_optimized,
        show_height
    );

    IMPLEMENT_JSON_SERIALIZATION(AppMemo,
        window_config,
        liveslam
    );
    
}

