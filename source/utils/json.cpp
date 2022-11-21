// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "json.h"

#include <boost/json/src.hpp>

namespace ds::utils
{

json::value read_json(const fs::path & path)
{
    std::ifstream is{path};
    if (!is.is_open())
        throw std::logic_error{std::format("Could not read file: {}", path.generic_string())};
    const std::string data{std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}};
    return json::parse(data);
}

} // namespace ds::utils
