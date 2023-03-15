// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "hugo.h"

#include "db/utils.h"
#include "formatter.h"
#include "utils/json.h"


namespace
{

constexpr const auto s_allowed_list = "ТОП-12 участников серии гран-при \"Стань чемпионом!\"";
constexpr const auto s_full_list = "Общий рейтинг участников серии гран-при \"Стань чемпионом!\"";
constexpr const auto s_rules_list = "Положение о соревнованиях серии гран-при \"Стань чемпионом!\"";
constexpr const auto s_competition_list = "Результаты прошедших турниров серии гран-при \"Стань чемпионом!\"";
constexpr const auto s_competition_results = "Результаты турнира: ";
constexpr const auto s_couples = "Пары";
constexpr const auto s_solo = "Соло";
constexpr const auto s_results = "Результаты";
constexpr const auto s_details = "Детализация";
constexpr const auto s_full_list_suffix = "-full-list";
constexpr const auto s_dancer_list_suffix = "-dancer-list";
constexpr const auto s_results_dir = "results";
constexpr const auto s_dancers_dir = "dancers";

} // namespace


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
    std::cout << fmt::format("Reading hugo config: {}\n", path.generic_string());

    try
    {
        const auto & json = utils::read_json(path);
        const auto & cfg = json.at("config");

        _accept_participants = cfg.at("accept_couples").as_int64();
        _title = cfg.at("title").as_string().c_str();
        _root_url = cfg.at("url").as_string().c_str();
        _banner = cfg.at("banner").as_string().c_str();
        _rules = cfg.at("rules").as_string().c_str();
        _export_all = cfg.at("export_all").as_bool();
        _export_details = cfg.at("export_details").as_bool();
        for (const auto & g : cfg.at("solo_groups").as_array())
            _solo_groups.emplace(g.as_string().c_str());
        for (const auto & g : cfg.at("couple_groups").as_array())
            _couple_groups.emplace(g.as_string().c_str());
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{fmt::format("Could not parse hugo config\nError: {}", ex.what())};
    }
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
        std::reverse(tokens.begin(), tokens.end());
        for (const auto & token : tokens)
            ss << token;
    }
    return ss.str();
}

std::string hugo::get_points_key(double points, const std::string & name1, const std::string & name2)
{
    points = 100000.0 - points * 100.0; // Some magic for string-based sorting in map
    std::string r =  fmt::format("{:.0f} {}", points, get_surname_key(name1, name2));
    return r;
}

void hugo::export_all(int64_t start_date, int64_t end_date)
{
    fs::create_directories(_output);

    const auto & comp_dir = _output / (_suffix + "results");
    fs::create_directories(comp_dir);

    std::stringstream extra;
    if (_export_all)
    {
        const auto & all_url = fmt::format("{}/{}{}", _root_url, _suffix, s_full_list_suffix);
        const auto & all_path = _output / fmt::format("{}{}.md", _suffix, s_full_list_suffix);
        export_full_list(all_path, start_date, end_date, all_url);

        formatter f{extra};
        f.h4(formatter::url(s_full_list, all_url));
    }
    if (_export_details)
    {
        const auto & dancers_url = fmt::format("{}/{}{}", _root_url, _suffix, s_dancer_list_suffix);
        const auto & dancers_path = _output / fmt::format("{}{}.md", _suffix, s_dancer_list_suffix);
        export_all_dancers(dancers_path, dancers_url, start_date, end_date);
        formatter f{extra};
        f.h4(formatter::url(s_details, dancers_url));
    }

    const auto & passed_path = _output / (_suffix + ".md");
    export_passed(passed_path, start_date, end_date, extra.str());
}

void hugo::export_passed(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & extra_header)
{
    export_custom(
        path,
        start_date,
        end_date,
        _title,
        _root_url,
        s_allowed_list,
        extra_header,
        true,
        true,
        true);
}

void hugo::export_full_list(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & url)
{
    std::stringstream extra;
    {
        const auto & competitions = _db.get_all<db::competition>(
            where(
                c(&db::competition::start_date) >= start_date
                and c(&db::competition::end_date) <= end_date));
        const auto & results_dir = _output / s_results_dir;
        fs::create_directories(results_dir);
        formatter f{extra};
        f.h3(s_competition_list);
        for (const auto & comp : competitions)
        {
            const auto name = fmt::format("{}-{}", comp.start_date, comp.end_date);
            const auto filepath = results_dir / fmt::format("{}.md", name);
            const auto comp_url = fmt::format("{}/{}/{}", _root_url, s_results_dir, name);
            export_competition(filepath, comp_url, comp);

            f.list(formatter::url(comp.title, comp_url));
        }
        f.br();
    }

    export_custom(
        path,
        start_date,
        end_date,
        _title,
        url,
        s_full_list,
        extra.str(),
        false,
        true,
        false);
}

void hugo::export_competition(const fs::path & path, const std::string & url, const db::competition & comp)
{
    const auto extra_header =
        fmt::format("{}\n\nГород: {}\n\nКоэффициент турнира: {:.1f}\n\nКоэффициент для иногородних участников: {:.1f}",
            formatter::url(s_results, comp.url),
            comp.host_city,
            comp.points_scale,
            comp.foreign_scale + 1.0);
    export_custom(
        path,
        comp.start_date,
        comp.end_date,
        comp.title,
        url,
        fmt::format("{}{}", s_competition_results, comp.title),
        extra_header,
        false,
        true,
        false);
}

void hugo::export_custom(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & title, const std::string & url, const std::string & header, const std::string & extra_header, bool only_passed_dancers, bool print_points, bool sort_points)
{
    std::cout << fmt::format("Exporting list of couples: {}\n", path.generic_string());
    std::ofstream os{path};
    if (!os.is_open())
        throw std::logic_error{fmt::format("Could not write file: {}", path.generic_string())};

    formatter f{os};

    f.yaml_header(title, url, "", _banner)
        .h2(header)
        .h4(formatter::url(s_rules_list, _rules))
        .br()
        .raw(extra_header)
        .br();

    const auto & groups = _db.get_all<db::group>(order_by(&db::group::min_year).desc());

    static const auto dancers_ids_from_points = [](const std::vector<db::bac_points> & input)
    {
        std::vector<int64_t> ids;
        ids.reserve(input.size());
        std::transform(input.begin(), input.end(), std::back_inserter(ids),
            [](const db::bac_points & s)
            {
                return s.dancer_id;
            });
        return ids;
    };

    // Couples
    {
        f.h3(s_couples);

        for (const auto & g : groups)
        {
            const auto & n = _db.get<db::group_name>(g.group_name_id);
            if (_couple_groups.find(n.name) == _couple_groups.end())
                continue;

            const auto & all_points = _db.get_all<db::bac_points>(
                where(
                    c(&db::bac_points::group_id) == g.id
                    and c(&db::bac_points::start_date) >= start_date
                    and c(&db::bac_points::end_date) <= end_date),
                order_by(&db::bac_points::points).desc());
            if (all_points.empty())
                continue;

            const auto pass_points = [&]()
            {
                auto passed = 0;
                auto last_pts = all_points[0].points;
                for (const auto & pts : all_points)
                {
                    if (passed == _accept_participants)
                        break;
                    if (last_pts != pts.points)
                        ++passed;
                    last_pts = pts.points;
                }
                return last_pts;
            }();

            const auto & dancers_ids = dancers_ids_from_points(all_points);
            const auto & couples = _db.get_all<db::couple>(
                where(
                    in(&db::couple::dancer_id1, dancers_ids)
                    or in(&db::couple::dancer_id2, dancers_ids)));

            std::map<std::string, std::tuple<db::dancer, db::bac_points, db::dancer, db::bac_points>> couples_sorted_by_name;
            std::map<std::string, std::tuple<db::dancer, db::bac_points, db::dancer, db::bac_points>> couples_sorted_by_points;
            for (const auto & cpl : couples)
            {
                if (cpl.is_solo)
                    continue;

                const auto & d1 = _db.get<db::dancer>(cpl.dancer_id1.value());
                const auto & d2 = _db.get<db::dancer>(cpl.dancer_id2.value());

                const auto & b_s1 = _db.get_all<db::bac_points>(
                    where(c(&db::bac_points::dancer_id) == d1.id
                        and c(&db::bac_points::group_id) == g.id
                        and c(&db::bac_points::start_date) == start_date
                        and c(&db::bac_points::end_date) == end_date));
                const auto & b_s2 = _db.get_all<db::bac_points>(
                    where(c(&db::bac_points::dancer_id) == d2.id
                        and c(&db::bac_points::group_id) == g.id
                        and c(&db::bac_points::start_date) == start_date
                        and c(&db::bac_points::end_date) == end_date));

                // Possible case when couple was changed between competitions
                if (b_s1.empty() || b_s2.empty())
                    continue;
                ds_assert(b_s1.size() == 1);
                ds_assert(b_s2.size() == 1);

                const auto points = std::min(b_s1[0].points, b_s2[0].points);
                if (only_passed_dancers)
                {
                    if (points < pass_points)
                        continue;
                }

                const auto name_key = get_surname_key(d1.name, d2.name);
                const auto points_key = get_points_key(points, d1.name, d2.name);
                const auto cpl_tuple = std::tuple{d1, b_s1[0], d2, b_s2[0]};
                couples_sorted_by_name[name_key] = cpl_tuple;
                couples_sorted_by_points[points_key] = cpl_tuple;
            }

            f.h4(n.title)
                .couples_header(print_points);

            for (const auto & it : (sort_points ? couples_sorted_by_points : couples_sorted_by_name))
            {
                const auto & [d1, b_s1, d2, b_s2] = it.second;
                if (print_points)
                    f.couple(d1.name, b_s1.points, d2.name, b_s2.points);
                else
                    f.couple(d1.name, d2.name);
            }

            f.table_footer();
        }
    }

    // Solo
    {
        f.h3(s_solo);

        for (const auto & g : groups)
        {
            const auto & n = _db.get<db::group_name>(g.group_name_id);
            if (_solo_groups.find(n.name) == _solo_groups.end())
                continue;

            const auto & all_points = _db.get_all<db::bac_points>(
                where(
                    c(&db::bac_points::group_id) == g.id
                    and c(&db::bac_points::start_date) >= start_date
                    and c(&db::bac_points::end_date) <= end_date),
                order_by(&db::bac_points::points).desc());
            if (all_points.empty())
                continue;

            const auto pass_points = [&]()
            {
                auto passed = 0;
                auto last_pts = all_points[0].points;
                for (const auto & pts : all_points)
                {
                    if (passed == _accept_participants)
                        break;
                    if (last_pts != pts.points)
                        ++passed;
                    last_pts = pts.points;
                }
                return last_pts;
            }();

            const auto & dancers_ids = dancers_ids_from_points(all_points);
            const auto & dancers = _db.get_all<db::dancer>(
                where(
                    in(&db::dancer::id, dancers_ids)));

            std::map<std::string, std::tuple<db::dancer, db::bac_points>> dancers_sorted_by_name;
            std::map<std::string, std::tuple<db::dancer, db::bac_points>> dancers_sorted_by_points;
            for (const auto & d : dancers)
            {
                const auto & b_s = _db.get_all<db::bac_points>(
                    where(c(&db::bac_points::dancer_id) == d.id
                        and c(&db::bac_points::group_id) == g.id
                        and c(&db::bac_points::start_date) == start_date
                        and c(&db::bac_points::end_date) == end_date));
                ds_assert(b_s.size() == 1);

                const auto points = b_s[0].points;
                if (only_passed_dancers)
                {
                    if (points < pass_points)
                        continue;
                }

                const auto name_key = get_surname_key(d.name);
                const auto points_key = get_points_key(points, d.name);
                const auto d_tuple = std::tuple{d, b_s[0]};
                dancers_sorted_by_name[name_key] = d_tuple;
                dancers_sorted_by_points[points_key] = d_tuple;
            }

            f.h4(n.title)
                .dancers_header(print_points);

            for (const auto & it : (sort_points ? dancers_sorted_by_points : dancers_sorted_by_name))
            {
                const auto & [d, b_s] = it.second;
                if (print_points)
                {
                    const auto points = b_s.points;
                    f.dancer(d.name, points);
                }
                else
                    f.dancer(d.name);
            }

            f.table_footer();
        }
    }
}

void hugo::export_all_dancers(const fs::path & path, const std::string & url, int64_t start_date, int64_t end_date)
{
    std::cout << fmt::format("Exporting list of dancers: {}\n", path.generic_string());
    std::ofstream os{path};
    if (!os.is_open())
        throw std::logic_error{fmt::format("Could not write file: {}", path.generic_string())};

    formatter f{os};
    f.yaml_header(s_details, url, "", "")
        .h2(s_details);

    const auto & competitions = _db.get_all<db::competition>(
        where(
            c(&db::competition::start_date) >= start_date
            and c(&db::competition::end_date) <= end_date));
    const auto & comp_ids = db::utils::ids(competitions);

    std::map<std::string, db::dancer> sorted;
    const auto & dancers = _db.get_all<db::dancer>();
    for (const auto & d : dancers)
    {
        const auto n_results = _db.count<db::bac_result>(
            where(
                c(&db::bac_result::dancer_id) == d.id
                and in(&db::bac_result::competition_id, comp_ids)),
            limit(1));
        if (n_results == 0)
            continue;
        if (!_export_all)
        {
            const auto & points = _db.get_all<db::bac_points>(
                where(
                    c(&db::bac_points::dancer_id) == d.id
                    and c(&db::bac_points::start_date) >= start_date
                    and c(&db::bac_points::end_date) <= end_date)
                // TODO: only top "_accept_couples" couples
            );
            if (points.empty())
                continue;
        }

        sorted[get_surname_key(d.name)] = d;
    }

    static const auto hash_dancer = [](const db::dancer & dancer)
    {
        static constexpr const auto salt = "b8c08c37-d12c-4989-bb92-bd97578fa7f3";
        const auto data = fmt::format("{}{}{}", salt, dancer.name, dancer.birthday);
        boost::uuids::detail::sha1 sha1;
        sha1.process_bytes(&data[0], data.size());
        boost::uuids::detail::sha1::digest_type d;
        sha1.get_digest(d);
        std::string result;
        const auto p = reinterpret_cast<const char *>(&d);
        boost::algorithm::hex(p, p + sizeof(d), std::back_inserter(result));
        boost::to_lower(result);
        return result;
    };

    f.h5(fmt::format("Всего участников: {}", sorted.size()));

    const auto dancers_dir = _output / s_dancers_dir;
    fs::create_directories(dancers_dir);
    for (const auto & it : sorted)
    {
        const auto & d = it.second;
        const auto hash = hash_dancer(d);
        const auto & dancer_url = fmt::format("{}/{}/{}", _root_url, s_dancers_dir, hash);
        const auto & dancer_path = dancers_dir / fmt::format("{}.md", hash);

        f.list(formatter::url(d.name, dancer_url));
        export_dancer(dancer_path, dancer_url, d, competitions, start_date, end_date);
    }
}

void hugo::export_dancer(const fs::path & path, const std::string & url, const db::dancer & dancer, const std::vector<db::competition> & competitions, int64_t start_date, int64_t end_date)
{
    std::ofstream os{path};
    if (!os.is_open())
        throw std::logic_error{fmt::format("Could not write file: {}", path.generic_string())};

    formatter f{os};
    f.yaml_header(dancer.name, url, "", "")
        .h2(dancer.name);

    const auto & groups = _db.get_all<db::group>();
    for (const auto & g : groups)
    {
        const auto & gr_name = _db.get<db::group_name>(g.group_name_id);
        f.h3(gr_name.title);

        f.raw("| Баллы | &nbsp;&nbsp;&nbsp; | Турнир |\n|--:|-|:--|\n");
        for (const auto & comp : competitions)
        {
            const auto & results = _db.get_all<db::bac_result>(
                where(
                    c(&db::bac_result::competition_id) == comp.id
                    and c(&db::bac_result::group_id) == g.id
                    and c(&db::bac_result::dancer_id) == dancer.id));
            if (results.empty())
                continue;
            const auto & points = _db.get_all<db::bac_points>(
                where(
                    c(&db::bac_points::group_id) == g.id
                    and c(&db::bac_points::dancer_id) == dancer.id
                    and c(&db::bac_points::start_date) >= comp.start_date
                    and c(&db::bac_points::end_date) <= comp.end_date));
            ds_assert(points.size() == 1);
            f.raw(fmt::format("|{:.1f}| &nbsp;&nbsp;&nbsp; |{}|", points[0].points, comp.title)).br();
        }

        const auto & total = _db.get_all<db::bac_points>(
            where(c(&db::bac_points::dancer_id) == dancer.id
                and c(&db::bac_points::group_id) == g.id
                and c(&db::bac_points::start_date) == start_date
                and c(&db::bac_points::end_date) == end_date));
        if (total.empty())
            continue;

        ds_assert(total.size() == 1);

        f.raw("| Итого: | &nbsp;&nbsp;&nbsp; |  |")
            .br()
            .raw(fmt::format("|{:.1f}| &nbsp;&nbsp;&nbsp; | |", total[0].points))
            .br(2);
    }
}


} // namespace ds::exp::hugo
