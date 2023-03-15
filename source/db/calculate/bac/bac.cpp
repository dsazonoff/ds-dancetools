// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "bac.h"

#include "db/utils.h"
#include "override_bac.h"

namespace ds::db
{

using namespace sqlite_orm;

bac::bac(const std::shared_ptr<db> & db)
    : base_logic{db}
    , _db_ptr{db}
{
}

void bac::evaluate(int64_t start_date, int64_t end_date, const fs::path& override_path) // NOLINT(misc-no-recursion)
{
    _ctx.competitions = _db.get_all<competition>(
        where(
            c(&competition::start_date) >= start_date
            and c(&competition::end_date) <= end_date));
    _ctx.groups = _db.get_all<group>();

    for (const auto & comp : _ctx.competitions)
        proceed_competition(comp);

    override_bac overrider{_db_ptr};
    overrider.set_config(override_path);
    overrider.apply(start_date, end_date);

    update_points(start_date, end_date);

    // Evaluate each competition
    for (const auto & comp : _ctx.competitions)
    {
        if (start_date == comp.start_date && end_date == comp.end_date)
            continue; // Skip recursion
        bac single_comp{_db_ptr};
        single_comp.evaluate(comp.start_date, comp.end_date, override_path);
    }
}

void bac::proceed_competition(const competition & comp)
{
    _ctx.competition = comp;
    _ctx.host_city_id = 0;
    const auto comp_city = _db.get_all<city>(
        where(c(&city::name) == _ctx.competition.host_city)
    );
    ds_assert(comp_city.size() <= 1);
    if (comp_city.size() == 1)
        _ctx.host_city_id = comp_city[0].id;
    for (const auto & group : _ctx.groups)
        proceed_group(group);
}

void bac::proceed_group(const group & gr)
{
    _ctx.group = gr;
    const auto & results = _db.get_all<result>(
        where(
            c(&result::competition_id) == _ctx.competition.id
            and c(&result::group_id) == _ctx.group.id),
        order_by(&result::place_start).asc());
    if (results.empty())
        return;

    const auto first_place = results.begin()->place_start;
    const auto last_place = results.rbegin()->place_end;

    if (first_place != 1
        || last_place != static_cast<int64_t>(results.size()))
    {
        const auto & comp_name = _ctx.competition.title;
        const auto & gr_name = _db.get<group_name>(gr.id).title;
        std::stringstream date;
        date << _ctx.competition.start_date;
        if (_ctx.competition.start_date != _ctx.competition.end_date)
            date << "-" << _ctx.competition.end_date;
        throw std::logic_error{fmt::format("Invalid results: {} | {} | {}", date.str(), comp_name, gr_name)};
    }

    constexpr const size_t n_places = 3;
    const auto n_results = results.size();
    size_t first_index = 0;
    for (size_t place = 1; place <= n_places; ++place)
    {
        // clang-format off
        const auto amount = static_cast<size_t>(std::ceil(
            static_cast<double>(n_results - first_index) /
            static_cast<double>(n_places - place + 1)));
        // clang-format on
        auto last_index = std::min(first_index + amount, n_results) - 1;
        for (auto index = last_index + 1; index < n_results; ++index)
        {
            const auto equal_results = results[index].place_start == results[last_index].place_start
                && results[index].place_end == results[last_index].place_end;
            if (!equal_results)
                break;
            last_index = index;
        }

        const auto points = std::round(static_cast<double>(n_places - place + 1));
        for (auto i = first_index; i <= last_index; ++i)
            proceed_result(results[i], place, points);

        first_index = last_index + 1;
        if (first_index >= results.size())
            break;
    }
}

void bac::proceed_result(const result & r, size_t place, double points)
{
    auto scale = _ctx.competition.points_scale;
    if (r.city_id != _ctx.host_city_id)
        scale += _ctx.competition.foreign_scale;
    points *= scale;

    const auto & cpl = _db.get<couple>(r.couple_id);
    for (const auto & d : {cpl.dancer_id1, cpl.dancer_id2})
    {
        if (!d.has_value())
            continue;

        const auto & b_results = _db.get_all<bac_result>(
            where(
                c(&bac_result::competition_id) == _ctx.competition.id
                and c(&bac_result::group_id) == _ctx.group.id
                and c(&bac_result::dancer_id) == d.value()));
        bac_result br = {
            0,
            _ctx.competition.id,
            _ctx.group.id,
            d.value(),
            static_cast<int64_t>(place),
            points};
        update_or_insert(b_results, br);
    }
}

void bac::update_points(int64_t start_date, int64_t end_date)
{
    const auto comp_ids = utils::ids(_ctx.competitions);
    for (const auto & comp : _ctx.competitions)
    {
        ds_assert(comp.start_date >= start_date);
        ds_assert(comp.end_date <= end_date);
        _ctx.competition = comp;
        for (const auto & group : _ctx.groups)
        {
            _ctx.group = group;
            const auto & all_results = _db.get_all<bac_result>(
                where(
                    c(&bac_result::competition_id) == _ctx.competition.id
                    and c(&bac_result::group_id) == _ctx.group.id));
            for (const auto & r : all_results)
            {
                const auto & d = _db.get<dancer>(r.dancer_id);
                const auto & b_points = _db.get_all<bac_points>(
                    where(
                        c(&bac_points::group_id) == _ctx.group.id
                        and c(&bac_points::dancer_id) == d.id
                        and c(&bac_points::start_date) == start_date
                        and c(&bac_points::end_date) == end_date));
                ds_assert(b_points.size() <= 1);
                bac_points b_s = {
                    0,
                    group.id,
                    d.id,
                    0.0,
                    start_date,
                    end_date};

                if (b_points.empty())
                    b_s.id = _db.insert(b_s);
                else
                    b_s = b_points[0];

                const auto & results = _db.get_all<bac_result>(
                    where(
                        c(&bac_result::group_id) == _ctx.group.id
                        and c(&bac_result::dancer_id) == d.id
                        and in(&bac_result::competition_id, comp_ids)));
                const auto points = std::accumulate(results.begin(), results.end(), 0.0, [](double v, const bac_result & b_r)
                    {
                        return v + b_r.points;
                    });
                b_s.points = points;
                _db.update(b_s);
            }
        }
    }
}

} // namespace ds::db
