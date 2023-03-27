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

cb::result ranking::result_callback()
{
    return [this](competition comp, group g, group_name gn, std::optional<dancer> d1, std::optional<dancer> d2, result r, city cc)
    {
        add_city(cc);
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

        add_dancer(d1);
        add_dancer(d2);

        couple cpl = {};
        if (d1.has_value())
            cpl.dancer_id1 = d1->id;
        if (d2.has_value())
            cpl.dancer_id2 = d2->id;
        cpl.is_solo = !cpl.dancer_id1.has_value() || !cpl.dancer_id2.has_value();
        add_couple(cpl);

        if (r.competition_id == 0)
            r.competition_id = comp.id;
        if (r.group_id == 0)
            r.group_id = g.id;
        if (r.couple_id == 0)
            r.couple_id = cpl.id;
        r.city_id = cc.id;
        add_result(r);
    };
}

cb::group_split ranking::split_callback()
{
    return [this](competition comp, group_name gn, std::vector<bac_group_split> splits)
    {
        add_competition(comp);
        const auto & names = _db.get_all<group_name>(where(c(&group_name::name) == gn.name));
        ds_assert(names.size() == 1);
        const auto & groups = _db.get_all<group>(where(c(&group::group_name_id) == names[0].id));
        ds_assert(groups.size() == 1);
        const auto & g = groups[0];

        for (auto & s : splits)
        {
            s.competition_id = comp.id;
            s.group_id = g.id;
            add_split(s);
        }
    };
}

void ranking::add_city(city & cc)
{
    const auto & cities = _db.get_all<city>(
        where(c(&city::name) == cc.name));
    get_or_insert(cities, cc);
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
            and c(&group::max_year) == g.max_year
            and c(&group::is_solo) == g.is_solo));
    get_or_insert(groups, g);
}

void ranking::add_dancer(std::optional<dancer> & d)
{
    if (!d.has_value())
        return;
    const auto & dancers = _db.get_all<dancer>(
        where(
            c(&dancer::name) == d->name
            and c(&dancer::birthday) == d->birthday
            and c(&dancer::bdsa_id) == d->bdsa_id));
    get_or_insert(dancers, d.value());
}

void ranking::add_couple(couple & cpl)
{
    const auto & couples = [&]()
    {
        if (cpl.is_solo)
            return _db.get_all<couple>(
                where(
                    (c(&couple::dancer_id1) == cpl.dancer_id1
                        and is_null(&couple::dancer_id2)
                        and c(&couple::is_solo) == true)
                    or (is_null(&couple::dancer_id1)
                        and c(&couple::dancer_id2) == cpl.dancer_id2
                        and c(&couple::is_solo) == true)));
        else
            return _db.get_all<couple>(
                where(
                    (c(&couple::dancer_id1) == cpl.dancer_id1
                        and c(&couple::dancer_id2) == cpl.dancer_id2
                        and c(&couple::is_solo) == false)));
    }();
    get_or_insert(couples, cpl);
}

void ranking::add_result(result & r)
{
    const auto & results = _db.get_all<result>(
        where(
            c(&result::competition_id) == r.competition_id
            and c(&result::group_id) == r.group_id
            and c(&result::couple_id) == r.couple_id));
    update_or_insert(results, r);
}

void ranking::add_split(bac_group_split & r)
{
    r.id = _db.insert(r);
}


} // namespace ds::db
