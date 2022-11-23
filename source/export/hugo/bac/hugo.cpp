// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "hugo.h"

#include "db/utils.h"
#include "fmt.h"
#include "utils/json.h"

namespace ds::exp::hugo
{

using namespace sqlite_orm;

hugo::hugo(const std::shared_ptr<db::db> & db)
    : base_logic{db}
{
}

void hugo::set_output_dir(fs::path dir)
{
    _output = std::move(dir);
}

void hugo::set_suffix(std::string suffix)
{
    _suffix = std::move(suffix);
}

void hugo::set_manifest(const fs::path & path)
{
    std::cout << std::format("Reading hugo config: {}\n", path.generic_string());

    try
    {
        const auto & json = utils::read_json(path);
        const auto & cfg = json.at("config");

        _minimum_points = cfg.at("minimum_points").as_double();
        _title = cfg.at("title").as_string().c_str();
        _root_url = cfg.at("url").as_string().c_str();
        _banner = cfg.at("banner").as_string().c_str();
        _rules = cfg.at("rules").as_string().c_str();
        for (const auto & g : cfg.at("solo_groups").as_array())
            _solo_groups.emplace(g.as_string().c_str());
        for (const auto & g : cfg.at("couple_groups").as_array())
            _couple_groups.emplace(g.as_string().c_str());
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{std::format("Could not parse hugo config\nError: {}", ex.what())};
    }
}

void hugo::export_all(int64_t start_date, int64_t end_date)
{
    fs::create_directories(_output);

    const auto & passed_path = _output / (_suffix + ".md");
    export_passed(passed_path, start_date, end_date);
}

void hugo::export_passed(const fs::path & path, int64_t start_date, int64_t end_date)
{
    std::cout << std::format("Exporting list of passed couples: {}", path.generic_string());
    std::ofstream os{path};
    if (!os.is_open())
        throw std::logic_error{std::format("Could not write file: {}", path.generic_string())};

    fmt f{os};
    // fmt f{std::cout};

    f.yaml_header(_title, _root_url, "", _banner)
        .h2("Список участников, допущенных к \"Альянс Трофи\"")
        .h5(fmt::url("Положение о соревнованиях серии Гран-при \"Стань чемпионом!\"", _rules));

    const auto & groups = _db.get_all<db::group>();
    const auto & competitions = _db.get_all<db::competition>(
        where(
            c(&db::competition::start_date) >= start_date
            and c(&db::competition::end_date) <= end_date));
    const auto comp_ids = db::utils::ids(competitions);

    // Couples
    {
        f.h3("Пары");

        for (const auto & g : groups)
        {
            const auto & n = _db.get<db::group_name>(g.group_name_id);
            if (!_couple_groups.contains(n.name))
                continue;

            const auto & all_stars = _db.get_all<db::bac_stars>(
                where(
                    c(&db::bac_stars::group_id) == g.id
                    and c(&db::bac_stars::start_date) >= start_date
                    and c(&db::bac_stars::end_date) <= end_date));
            if (all_stars.empty())
                continue;
            const auto star_ids = db::utils::ids(all_stars);

            const auto & couples = _db.get_all<db::couple>(
                where(
                    in(&db::couple::dancer_id1, star_ids)
                    or in(&db::couple::dancer_id2, star_ids)));

            std::map<std::string, std::tuple<db::dancer, db::bac_stars, db::dancer, db::bac_stars>> couple_sorted;
            for (const auto & cpl : couples)
            {
                if (cpl.is_solo)
                    continue;

                const auto & d1 = _db.get<db::dancer>(cpl.dancer_id1.value());
                const auto & b_s1 = _db.get<db::bac_stars>(d1.id);
                const auto & d2 = _db.get<db::dancer>(cpl.dancer_id2.value());
                const auto & b_s2 = _db.get<db::bac_stars>(d2.id);
                couple_sorted[get_surname_key(d1.name, d2.name)] = std::tuple{d1, b_s1, d2, b_s2};
            }

            f.h4(n.title)
                .couples_header();

            for(const auto & it : couple_sorted)
            {
                const auto & [d1, b_s1, d2, b_s2] = it.second;
                const auto stars = std::max(b_s1.stars, b_s2.stars);
                f.couple(d1.name, b_s1.stars, d2.name, b_s2.stars);
            }

            f.table_footer();
        }
    }

    // Solo
}

std::string hugo::get_surname_key(const std::string & name1, const std::string & name2)
{
    std::stringstream ss;
    for (const auto & n : {name1, name2})
    {
        if (n.empty())
            continue;
        std::vector<std::string> tokens;
        tokens.reserve(2);
        boost::split(tokens, n, boost::is_any_of(" "));
        for (const auto & token : std::ranges::reverse_view(tokens))
            ss << token;
    }
    return ss.str();
}


} // namespace ds::exp::hugo
