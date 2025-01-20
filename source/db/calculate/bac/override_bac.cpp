// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "override_bac.h"

#include "db/utils.h"
#include "utils/json.h"

namespace ds::db
{

using namespace sqlite_orm;


override_bac::override_bac(const std::shared_ptr<db> & db)
    : base_logic{db}
{
}

void override_bac::set_config(const fs::path & path)
{
    _rules.clear();
    if (!fs::exists(path))
        return;

    try
    {
        std::cout << fmt::format("Parsing override config: {}\n", path.generic_string());
        const auto & json = ds::utils::read_json(path);
        const auto dancers = json.at("dancers").as_array();
        _rules.reserve(dancers.size());
        for (const auto & obj : dancers)
        {
            const std::string action = obj.at("action").as_string().c_str();
            if (action == "move")
            {
                move_dancer data = {
                    obj.at("name").as_string().c_str(),
                    obj.at("from_group").as_string().c_str(),
                    obj.at("to_group").as_string().c_str(),
                };
                _rules.emplace_back(data);
                continue;
            }
            if (action == "remove")
            {
                remove_dancer data = {
                    obj.at("name").as_string().c_str(),
                    obj.at("from_group").as_string().c_str(),
                    obj.at("start_date").as_int64(),
                    obj.at("end_date").as_int64(),
                };
                _rules.emplace_back(data);
                continue;
            }
            if (action == "add_points")
            {
                add_points data = {
                    obj.at("name").as_string().c_str(),
                    obj.at("group").as_string().c_str(),
                    obj.at("start_date").as_int64(),
                    obj.at("end_date").as_int64(),
                    obj.at("points").as_double(),
                };
                _rules.emplace_back(data);
                continue;
            }
        }
        const auto couples = json.at("couples").as_array();
        _rules.reserve(_rules.size() + couples.size());
        for (const auto & obj : couples)
        {
            const std::string action = obj.at("action").as_string().c_str();
            if (action == "remove")
            {
                remove_couple data = {
                    obj.at("male").as_string().c_str(),
                    obj.at("female").as_string().c_str(),
                    obj.at("from_group").as_string().c_str(),
                };
                _rules.emplace_back(data);
            }
        }
    }
    catch (const std::exception & ex)
    {
        throw std::logic_error{fmt::format("Could not parse override config: {}\n{}", path.generic_string(), ex.what())};
    }
}

template<class... Ts> struct overloaded: Ts...
{
    using Ts::operator()...;
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void override_bac::apply(int64_t start_date, int64_t end_date)
{
    for (const auto & r : _rules)
        std::visit(overloaded{
                       [&](const move_dancer & data)
                       {
                           on_move(data, start_date, end_date);
                       },
                       [&](const remove_dancer & data)
                       {
                           on_remove(data, start_date, end_date);
                       },
                       [&](const add_points & data)
                       {
                           on_add_points(data, start_date, end_date);
                       },
                       [&](const remove_couple & data)
                       {
                           on_remove(data, start_date, end_date);
                       },
                   },
            r);
}

void override_bac::on_move(const override_bac::move_dancer & data, int64_t start_date, int64_t end_date)
{
    const auto & gn_from = _db.get_all<group_name>(
        where(c(&group_name::name) == data.from_group));
    const auto & gn_to = _db.get_all<group_name>(
        where(c(&group_name::name) == data.to_group));
    ds_assert(gn_from.size() == 1);
    ds_assert(gn_to.size() == 1);
    const auto & g_from = _db.get_all<group>(
        where(c(&group::group_name_id) == gn_from[0].id));
    const auto & g_to = _db.get_all<group>(
        where(c(&group::group_name_id) == gn_to[0].id));
    ds_assert(g_from.size() == 1);
    ds_assert(g_to.size() == 1);
    const auto & from = g_from[0];
    const auto & to = g_to[0];

    const auto & competitions = _db.get_all<competition>(
        where(
            c(&competition::start_date) >= start_date
            and c(&competition::end_date) <= end_date));
    const auto comp_ids = utils::ids(competitions);

    const auto & dancers = _db.get_all<dancer>(
        where(c(&dancer::name) == data.name));
    if (dancers.empty())
        return;

    std::cout << fmt::format("Overriding (moving) results for: {} | {} -> {}\n", data.name, gn_from[0].title, gn_to[0].title);

    const auto & d_ids = utils::ids(dancers);
    const auto & results = _db.get_all<bac_result>(
        where(
            c(&bac_result::group_id) == from.id
            and in(&bac_result::dancer_id, d_ids)
            and in(&bac_result::competition_id, comp_ids)));
    for (const auto & r : results)
    {
        auto up = r;
        up.group_id = to.id;
        _db.update(up);
    }
}

void override_bac::on_remove(const override_bac::remove_dancer & data, int64_t start_date, int64_t end_date)
{
    if (data.start_date < start_date || data.end_date > end_date)
        return;

    const auto & gn_from = _db.get_all<group_name>(
        where(c(&group_name::name) == data.from_group));
    ds_assert(gn_from.size() == 1);
    const auto & g_from = _db.get_all<group>(
        where(c(&group::group_name_id) == gn_from[0].id));
    ds_assert(g_from.size() == 1);
    const auto & from = g_from[0];

    const auto & competitions = _db.get_all<competition>(
        where(
            c(&competition::start_date) >= data.start_date
            and c(&competition::end_date) <= data.end_date));
    const auto comp_ids = utils::ids(competitions);

    const auto & dancers = _db.get_all<dancer>(
        where(c(&dancer::name) == data.name));
    if (dancers.empty())
        return;

    std::cout << fmt::format("Overriding (removing) results for: {} | {}\n", data.name, gn_from[0].title);

    const auto & d_ids = utils::ids(dancers);
    _db.remove_all<bac_result>(
        where(
            c(&bac_result::group_id) == from.id
            and in(&bac_result::dancer_id, d_ids)
            and in(&bac_result::competition_id, comp_ids)));
    _db.remove_all<bac_points>(
        where(
            c(&bac_points::group_id) == from.id
            and in(&bac_points::dancer_id, d_ids)
            and c(&bac_points::start_date) >= start_date
            and c(&bac_points::end_date) <= end_date));
}

void override_bac::on_add_points(const override_bac::add_points & data, int64_t start_date, int64_t end_date)
{
    (void)start_date;
    (void)end_date;
    const auto & gn = _db.get_all<group_name>(
        where(c(&group_name::name) == data.group));
    ds_assert(gn.size() == 1);
    const auto & g = _db.get_all<group>(
        where(c(&group::group_name_id) == gn[0].id));
    ds_assert(g.size() == 1);
    const auto & from = g[0];

    const auto & competitions = _db.get_all<competition>(
        where(
            c(&competition::start_date) == data.start_date
            and c(&competition::end_date) == data.end_date));
    if (competitions.empty())
        return;
    ds_assert(competitions.size() == 1);
    const auto & comp = competitions[0];

    const auto & dancers = _db.get_all<dancer>(
        where(c(&dancer::name) == data.name));
    if (dancers.empty())
        return;

    std::cout << fmt::format("Overriding (adding points: {}) results for: {} | {} | {}\n", data.points, data.name, gn[0].title, comp.title);

    const auto & d_ids = utils::ids(dancers);
    const auto & results = _db.get_all<bac_result>(
        where(
            c(&bac_result::group_id) == from.id
            and in(&bac_result::dancer_id, d_ids)
            and c(&bac_result::competition_id) == comp.id));
    ds_assert(results.size() == 1);
    auto r = results[0];
    r.points += data.points;
    _db.update(r);
}

void override_bac::on_remove(const override_bac::remove_couple & data, int64_t start_date, int64_t end_date)
{
    const auto & gn_from = _db.get_all<group_name>(
        where(c(&group_name::name) == data.from_group));
    ds_assert(gn_from.size() == 1);
    //    const auto & g_from = _db.get_all<group>(
    //        where(c(&group::group_name_id) == gn_from[0].id));
    //    ds_assert(g_from.size() == 1);
    //    const auto & from = g_from[0];

    const auto & competitions = _db.get_all<competition>(
        where(
            c(&competition::start_date) >= start_date
            and c(&competition::end_date) <= end_date));
    const auto comp_ids = utils::ids(competitions);

    const auto & dancers = _db.get_all<dancer>(
        where(
            c(&dancer::name) == data.male
            or c(&dancer::name) == data.female));

    if (dancers.empty())
        return;

    std::cout << fmt::format("!!NOT IMPLEMENTED! Overriding (removing) results for: {} / {} | {}\n", data.male, data.female, gn_from[0].title);

    // TODO: implement

    //    const auto & d_ids = utils::ids(dancers);
    //    _db.remove_all<bac_result>(
    //        where(
    //            c(&bac_result::group_id) == from.id
    //            and in(&bac_result::dancer_id, d_ids)
    //            and in(&bac_result::competition_id, comp_ids)));
    //
    //    const auto & cpl = _db.get_all<couple>(
    //        where(
    //            is_not_null(&couple::dancer_id1)
    //            and in(&couple::dancer_id1, d_ids)
    //            and is_not_null(&couple::dancer_id2)
    //            and in(&couple::dancer_id2, d_ids)
    //            and c(&couple::is_solo) == false));
    //
    //    _db.remove_all<couple>(
    //        where(
    //            is_not_null(&couple::dancer_id1)
    //            and in(&couple::dancer_id1, d_ids)
    //            and is_not_null(&couple::dancer_id2)
    //            and in(&couple::dancer_id2, d_ids)
    //            and c(&couple::is_solo) == false));
}

} // namespace ds::db
