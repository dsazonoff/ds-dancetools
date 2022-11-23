// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "bac.h"

namespace ds::db
{

using namespace sqlite_orm;

bac::bac(const std::shared_ptr<db> & db)
    : base_logic{db}
{
}

void bac::evaluate(int64_t start_date, int64_t end_date)
{
    _ctx = {};
    _ctx.competitions = _db.get_all<competition>(
        where(
            c(&competition::start_date) >= start_date
            and c(&competition::end_date) <= end_date));
    _ctx.groups = _db.get_all<group>();

    for (const auto & comp : _ctx.competitions)
        proceed_competition(comp);

    update_stars(start_date, end_date);
}

void bac::proceed_competition(const competition & comp)
{
    _ctx.competition = comp;
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

    ds_assert(results.begin()->place_start == 1);
    ds_assert(results.rbegin()->place_start == static_cast<int64_t>(results.size()));

    constexpr const size_t n_places = 3;
    const auto n_results = results.size();
    const auto amount = static_cast<size_t>(std::ceil(static_cast<double>(n_results) / n_places));
    size_t first_index = 0;
    for (size_t place = 1; place <= n_places; ++place)
    {
        auto last_index = std::min(first_index + amount, n_results) - 1;
        for (auto index = last_index + 1; index < n_results; ++index)
        {
            const auto equal_results = results[index].place_start == results[last_index].place_start
                && results[index].place_end == results[last_index].place_end;
            if (!equal_results)
                break;
            last_index = index;
        }

        const auto stars = std::round(static_cast<double>(n_places - place + 1) * _ctx.competition.points_scale);
        for (auto i = first_index; i <= last_index; ++i)
            proceed_result(results[i], place, stars);

        first_index = last_index + 1;
        if (first_index >= results.size())
            break;
    }
}

void bac::proceed_result(const result & r, size_t place, double stars)
{
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
            stars};
        update_or_insert(b_results, br);
    }
}

void bac::update_stars(int64_t start_date, int64_t end_date)
{
    for (const auto & comp : _ctx.competitions)
    {
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
                const auto & b_stars = _db.get_all<bac_stars>(
                    where(
                        c(&bac_stars::group_id) == _ctx.group.id
                        and c(&bac_stars::dancer_id) == d.id));
                ds_assert(b_stars.size() <= 1);
                bac_stars b_s = {
                    0,
                    group.id,
                    d.id,
                    0.0,
                    start_date,
                    end_date};

                if (b_stars.empty())
                    b_s.id = _db.insert(b_s);
                else
                    b_s = b_stars[0];

                const auto & results = _db.get_all<bac_result>(
                    where(
                        c(&bac_result::competition_id) == _ctx.competition.id
                        and c(&bac_result::group_id) == _ctx.group.id
                        and c(&bac_result::dancer_id) == d.id));
                const auto stars = std::accumulate(results.begin(), results.end(), 0.0, [](double v, const bac_result & b_r)
                    {
                        return v + b_r.stars;
                    });
                b_s.stars = stars;
                _db.update(b_s);
            }
        }
    }
}

} // namespace ds::db
