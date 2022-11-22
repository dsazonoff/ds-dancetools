// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "ranking.h"


namespace ds::db
{

using namespace sqlite_orm;

ranking::ranking(const std::shared_ptr<db> & db)
    : base_logic{db}
{
}

cb::result ranking::callback()
{
    return [this](competition comp, group g, group_name gn, couple cpl, result r)
    {
        add_competition(comp);

        if (gn.id == 0)
        {
            const auto & names = _db.get_all<group_name>(where(c(&group_name::name) == gn.name));
            ds_assert(names.size() == 1);
            gn.id = names[0].id;
        }
        if (g.group_name_id == 0)
            g.group_name_id = gn.id;
        add_group(g);

        ds_assert(g.is_solo == cpl.is_solo);
        add_couple(cpl);

        if (r.competition_id == 0)
            r.competition_id = comp.id;
        if (r.group_id == 0)
            r.group_id = g.id;
        if (r.couple_id == 0)
            r.couple_id = cpl.id;
        add_result(r);
    };
}

void ranking::add_competition(competition & comp)
{
    const auto & competitions = _db.get_all<competition>(
        where(
            c(&competition::title) == comp.title
            and c(&competition::start_date) == comp.start_date
            and c(&competition::end_date) == comp.end_date));
    get_or_insert(competitions, comp);
}

void ranking::add_group(group & g)
{
    const auto & groups = _db.get_all<group>(
        where(
            c(&group::group_name_id) == g.group_name_id
            and c(&group::min_year) == g.min_year
            and c(&group::max_year) == g.max_year));
    get_or_insert(groups, g);
}

void ranking::add_couple(couple & cpl)
{
    const auto & couples = _db.get_all<couple>(
        where(
            c(&couple::bdsa_id1) == cpl.bdsa_id1
            and c(&couple::name1) == cpl.name1
            and c(&couple::surname1) == cpl.surname1
            and c(&couple::bdsa_id1) == cpl.bdsa_id1
            and c(&couple::name2) == cpl.name2
            and c(&couple::surname2) == cpl.surname2));
    get_or_insert(couples, cpl);
}

void ranking::add_result(result & r)
{
    const auto& results = _db.get_all<result>(
        where(
            c(&result::competition_id) == r.competition_id
            and c(&result::group_id) == r.group_id
            and c(&result::couple_id) == r.couple_id
            )
        );
    ds_assert(results.size() <= 1);
    if (results.empty())
    {
        r.id = _db.insert(r);
        return;
    }
    r.id = results[0].id;
    _db.update(r);
}


} // namespace ds::db
