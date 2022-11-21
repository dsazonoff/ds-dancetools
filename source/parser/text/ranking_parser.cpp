// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "ranking_parser.h"

#include "utils/json.h"


namespace
{
constexpr const auto s_manifest = "manifest.json";
}

namespace ds::parser
{

ranking_parser::ranking_parser() = default;

void ranking_parser::set_root_dir(fs::path dir)
{
    ds_assert(fs::is_directory(dir));
    _root = std::move(dir);
}

void ranking_parser::set_callback(result_cb callback)
{
    _result_callback = std::move(callback);
}

void ranking_parser::parse()
{
    _ctx = {};
    for (const auto & entry : fs::directory_iterator(_root))
    {
        const auto & path = entry.path().filename();
        if (!fs::is_directory(_root / path))
            continue;

        parse_results_dir(path);
    }
}

void ranking_parser::parse_results_dir(const fs::path & results_dir)
{
    _ctx.working_dir = _root / results_dir;
    const auto & manifest_path = _ctx.working_dir / s_manifest;
    std::cout << "Reading manifest: " << manifest_path.generic_string() << std::endl;
    _ctx.manifest = utils::read_json(manifest_path);
    _ctx.competition = parse_competition(_ctx.manifest);
    _ctx.groups = parse_groups(_ctx.manifest);

    for (const auto & entry : fs::directory_iterator(_ctx.working_dir))
    {
        const auto & path = entry.path();
        if (path.extension() != ".txt")
            continue;

        parse_results_file(path);
    }
}

db::competition ranking_parser::parse_competition(const json::value & manifest) const
{
    db::competition c = {};
    try
    {
        const auto & obj = manifest.at("competition");
        const std::string title = obj.at("title").as_string().c_str();
        const std::string start_date = obj.at("start_date").as_string().c_str();
        const std::string end_date = obj.at("end_date").as_string().c_str();
        const std::string url = obj.at("url").as_string().c_str();
        const double points_scale = obj.at("points_scale").as_double();

        const auto parse_date = [](const std::string & date)
        {
            std::vector<std::string> items;
            items.reserve(3);
            boost::split(items, date, boost::is_any_of("-"));
            if (items.size() != 3)
                throw std::logic_error{std::format("Could not parse date: {}", date)};
            const auto year = boost::lexical_cast<int64_t>(items[0]);
            const auto month = boost::lexical_cast<int64_t>(items[1]);
            const auto day = boost::lexical_cast<int64_t>(items[2]);
            const auto result = year * 10000 + month * 100 + day;
            return result;
        };

        c.title = title;
        c.start_date = parse_date(start_date);
        c.end_date = parse_date(end_date);
        c.url = url;
        c.points_scale = points_scale;
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{std::format("Could not parse manifest: {}\nError: {}", _ctx.working_dir.generic_string(), ex.what())};
    }

    return c;
}

std::map<std::string, db::group> ranking_parser::parse_groups(const json::value & manifest) const
{
    std::map<std::string, db::group> g;
    try
    {
        const auto & groups = manifest.at("groups").as_array();
        for (const auto & obj : groups)
        {
            const std::string id_name = obj.at("id").as_string().c_str();
            const auto min_year = obj.at("min_year").as_int64();
            const auto max_year = obj.at("max_year").as_int64();
            const auto is_solo = obj.at("solo").as_bool();
            g.emplace(id_name, db::group{0, id_name, min_year, max_year, is_solo});
        }
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{std::format("Could not parse manifest: {}\nError: {}", _ctx.working_dir.generic_string(), ex.what())};
    }

    return g;
}


void ranking_parser::parse_results_file(const fs::path & path)
{
    std::cout << "Parsing file: " << path.generic_string() << std::endl;
    const auto & group_id = path.stem().generic_string();
    const auto group_it = _ctx.groups.find(group_id);
    if (group_it == _ctx.groups.end())
        throw std::logic_error{std::format("Group is not in manifest: {}", group_id)};
    _ctx.group = group_it->second;

    std::ifstream is{path.generic_string()};
    if (!is.is_open())
        throw std::logic_error{std::format("Could not read file: {}", path.generic_string())};

    _ctx.file = path;
    for (std::string line; !!std::getline(is, line);)
    {
        auto [couple, result] = parse_line(line);
        _result_callback(_ctx.competition, _ctx.group, std::move(couple), std::move(result));
    }
}

std::tuple<db::couple, db::result> ranking_parser::parse_line(const std::string & line) const
{
    db::couple couple = {};
    db::result result = {};

    const auto parse_place = [](const std::string & text)
    {
        db::result r = {};
        std::vector<std::string> words;
        boost::split(words, text, boost::is_any_of(" .-"));

        switch(words.size())
        {
        case 2:
            r.place_start = boost::lexical_cast<int64_t>(words[0]);
            r.place_end = r.place_start;
            break;
        case 5:
            r.place_start = boost::lexical_cast<int64_t>(words[0]);
            r.place_end = boost::lexical_cast<int64_t>(words[3]);
            break;
        default:
            throw std::logic_error{"place"};
        }
        return r;
    };

    const auto parse_names = [](const std::string & text)
    {
        db::couple c = {};
        std::vector<std::string> words;
        boost::split(words, text, boost::is_any_of(" /"));
        if (words.size() != 6)
            throw std::logic_error{""};
        std::for_each(words.begin(), words.end(), [](auto & w)
            {
                if (w == "-")
                    w.clear();
            });
        c.name1 = std::move(words[0]);
        c.surname1 = std::move(words[1]);
        c.name2 = std::move(words[4]);
        c.surname2 = std::move(words[5]);
        if (c.name1.empty() || c.name2.empty())
        {
            c.is_solo = true;
            if (c.name1.empty())
            {
                c.name1 = std::move(c.name2);
                c.surname1 = std::move(c.surname2);
            }
        }
        return c;
    };

    try
    {
        std::vector<std::string> tokens;
        tokens.reserve(4);
        boost::split(tokens, line, boost::is_any_of("\t"));
        if (tokens.size() != 4)
            throw std::logic_error{"names"};

        const auto & place_text = tokens[0];
        const auto & couple_text = tokens[2];

        result = parse_place(place_text);
        couple = parse_names(couple_text);
    }
    catch (const std::logic_error &ex)
    {
        throw std::logic_error{std::format("Could not parse line: {} - {}\nFile: {}", line, ex.what(), _ctx.file.generic_string())};
    }

    return std::tuple{couple, result};
}


} // namespace ds::parser
