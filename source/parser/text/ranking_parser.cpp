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

void ranking_parser::set_callback(db::cb::result result_callback, db::cb::group_split split_callback)
{
    _result_callback = std::move(result_callback);
    _split_callback = std::move(split_callback);
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
    std::cout << fmt::format("Reading manifest: {}\n", manifest_path.generic_string());
    _ctx.manifest = utils::read_json(manifest_path);
    _ctx.competition = parse_competition(_ctx.manifest);
    _ctx.groups = parse_groups(_ctx.manifest);
    _ctx.group_results = parse_group_results(_ctx.manifest);

    for (const auto & entry : fs::directory_iterator(_ctx.working_dir))
    {
        const auto & path = entry.path();
        if (path.extension() != ".txt")
            continue;

        parse_results_file(path);
    }

    ds_assert(_split_callback);
    for (const auto & [group_id, split] : _ctx.group_results)
    {
        _split_callback(_ctx.competition, db::group_name{0, group_id, {}}, split);
    }

    std::cout << "\n";
}

db::competition ranking_parser::parse_competition(const json::value & manifest) const
{
    try
    {
        db::competition c = {};

        const auto & obj = manifest.at("competition");
        const std::string title = obj.at("title").as_string().c_str();
        const std::string start_date = obj.at("start_date").as_string().c_str();
        const std::string end_date = obj.at("end_date").as_string().c_str();
        const std::string url = obj.at("url").as_string().c_str();
        const std::string host_city = obj.at("host_city").as_string().c_str();
        const double points_scale = obj.at("points_scale").as_double();
        const double foreign_scale = obj.at("foreign_scale").as_double();

        const auto parse_date = [](const std::string & date)
        {
            std::vector<std::string> items;
            items.reserve(3);
            boost::split(items, date, boost::is_any_of("-"));
            if (items.size() != 3)
                throw std::logic_error{fmt::format("Could not parse date: {}", date)};
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
        c.host_city = host_city;
        c.points_scale = points_scale;
        c.foreign_scale = foreign_scale;

        if (c.start_date > c.end_date)
            throw std::logic_error{fmt::format("start_date should be lower than end_date")};

        return c;
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{fmt::format("Could not parse manifest: {}\nError: {}", _ctx.working_dir.generic_string(), ex.what())};
    }
}

std::map<std::string, db::group> ranking_parser::parse_groups(const json::value & manifest) const
{
    try
    {
        std::map<std::string, db::group> g;

        const auto & groups = manifest.at("groups").as_array();
        for (const auto & obj : groups)
        {
            const std::string id_name = obj.at("id").as_string().c_str();
            const auto min_year = obj.at("min_year").as_int64();
            const auto max_year = obj.at("max_year").as_int64();
            const auto is_solo = obj.at("solo").as_bool();
            g.try_emplace(id_name, db::group{0, 0, min_year, max_year, is_solo});
        }

        return g;
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{fmt::format("Could not parse manifest: {}\nError: {}", _ctx.working_dir.generic_string(), ex.what())};
    }
}

std::map<std::string, std::vector<db::bac_group_split>> ranking_parser::parse_group_results(const json::value & manifest) const
{
    try
    {
        std::map<std::string, std::vector<db::bac_group_split>> g;

        const auto & groups = manifest.at("groups").as_array();
        for (const auto & obj : groups)
        {
            const std::string id_name = obj.at("id").as_string().c_str();
            const auto & results = obj.at("split");

            std::vector<db::bac_group_split> gr;
            int64_t place = 1;
            for (const auto & r : results.as_array())
            {
                gr.emplace_back(db::bac_group_split{0, 0, 0, place, r.as_int64()});
                ++place;
            }

            g.try_emplace(id_name, std::move(gr));
        }

        return g;
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{fmt::format("Could not parse manifest: {}\nError: {}", _ctx.working_dir.generic_string(), ex.what())};
    }
}

void ranking_parser::parse_results_file(const fs::path & path)
{
    std::cout << fmt::format("Parsing file: {}\n", path.generic_string());
    const auto & group_id = path.stem().generic_string();
    const auto group_it = _ctx.groups.find(group_id);
    if (group_it == _ctx.groups.end())
        throw std::logic_error{fmt::format("Group is not in manifest: {}", group_id)};
    const auto & group_name = group_it->first;
    _ctx.group = group_it->second;

    std::ifstream is{path.generic_string()};
    if (!is.is_open())
        throw std::logic_error{fmt::format("Could not read file: {}", path.generic_string())};

    ds_assert(_result_callback);
    _ctx.file = path;
    for (std::string line; !!std::getline(is, line);)
    {
        auto [dancer1, dancer2, result, city] = parse_line(line);
        _result_callback(_ctx.competition, _ctx.group, db::group_name{0, group_name, std::string{}}, std::move(dancer1), std::move(dancer2), result, std::move(city));
    }
}

std::tuple<std::optional<db::dancer>, std::optional<db::dancer>, db::result, db::city> ranking_parser::parse_line(const std::string & line) const
{
    static const auto parse_place = [](const std::string & text)
    {
        db::result r = {};
        std::vector<std::string> words;
        boost::split(words, text, boost::is_any_of(" .-"));

        switch (words.size())
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
        if (r.place_start > r.place_end)
            throw std::logic_error{"Invalid place"};

        return r;
    };

    static const auto parse_names = [](const std::string & text)
    {
        std::optional<db::dancer> d1 = db::dancer{};
        std::optional<db::dancer> d2 = db::dancer{};

        std::vector<std::string> names;
        split_regex(names, text, boost::regex(" / "));
        if (names.size() != 2)
            throw std::logic_error{fmt::format("Invalid names: {}", text)};

        static const auto get_fio = [](const std::string& token)
        {
            if (token == "- -")
                return std::tuple<std::string, std::string>{};

            const auto pos = token.rfind(' ');
            if (pos == std::string::npos)
                throw std::logic_error{fmt::format("Invalid name: {}", token)};
            const auto& name = token.substr(0, pos);
            const auto& surname = token.substr(pos + 1);

            return std::tuple{name, surname};
        };

        const auto t1 = get_fio(names[0]);
        const auto t2 = get_fio(names[1]);
        d1->name = fmt::format("{} {}", std::get<1>(t1), std::get<0>(t1));
        d2->name = fmt::format("{} {}", std::get<1>(t2), std::get<0>(t2));

        if (d1->name == " ")
            d1 = std::nullopt;
        if (d2->name == " ")
            d2 = std::nullopt;
        ds_assert(d1.has_value() || d2.has_value());

        return std::tuple(d1, d2);
    };

    static const auto parse_city = [](const std::string & text)
    {
        std::vector<std::string> words;
        boost::algorithm::split_regex(words, text, boost::regex(" / "));
        ds_assert(!words.empty());
        const auto n_separators = std::count(text.begin(), text.end(), '/');
        const auto city_index = [&]() -> size_t
        {
            switch (n_separators)
            {
            case 0:
                return 0; // city
            case 1:
                return 1; // country / city
            case 2:
                return 0; // city / club / coaches
            case 3:
                return 1; // country / city / club / coaches
            default:
                break;
            }
            throw std::logic_error{"country/city/club/coach"};
        }();

        db::city c = {};
        c.name = words[city_index];

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
        const auto result = parse_place(place_text);
        auto [d1, d2] = parse_names(couple_text);
        auto city = parse_city(tokens[3]);

        return std::tuple{std::move(d1), std::move(d2), result, std::move(city)};
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{fmt::format("Could not parse line: \"{}\" - {}\nFile: {}", line, ex.what(), _ctx.file.generic_string())};
    }
}


} // namespace ds::parser
