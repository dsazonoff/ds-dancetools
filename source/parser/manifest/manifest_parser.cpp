// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "manifest_parser.h"

#include "utils/json.h"

namespace
{
constexpr const auto s_manifest = "manifest.json";
}


namespace ds::parser
{

manifest_parser::manifest_parser() = default;


void manifest_parser::set_root_dir(fs::path dir)
{
    ds_assert(fs::is_directory(dir));
    _root = std::move(dir);
}

void manifest_parser::set_group_callback(db::cb::group_name callback)
{
    _group_callback = std::move(callback);
}

void manifest_parser::parse()
{
    const auto & manifest_path = _root / s_manifest;
    std::cout << "Reading manifest: " << manifest_path.generic_string() << std::endl;

    try
    {
        const auto json = utils::read_json(manifest_path);
        const auto & groups = json.at("groups").as_array();
        for (const auto & group : groups)
        {
            const std::string id = group.at("id").as_string().c_str();
            const std::string title = group.at("title").as_string().c_str();

            db::group_name g = {0, id, title};
            if (_group_callback)
                _group_callback(std::move(g));
        }
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{std::format("Could not parse manifest: {}\nError: {}", manifest_path.generic_string(), ex.what())};
    }
}


} // namespace ds::parser
