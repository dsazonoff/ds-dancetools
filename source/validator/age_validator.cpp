// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "age_validator.h"

#include "db/utils.h"

namespace ds::validator
{

age_validator::age_validator(const std::shared_ptr<db::db> & db)
    : base_logic{db}
{
}

void age_validator::validate()
{
    using namespace sqlite_orm;

    const auto & solo_groups = _db.get_all<db::group>(
        where(c(&db::group::is_solo) == true));
    const auto & couple_groups = _db.get_all<db::group>(
        where(c(&db::group::is_solo) == false));

    std::vector<int64_t> dancer_ids;
    for (const auto & d : _db.iterate<db::dancer>())
    {
        std::set<int64_t> solo_group_ids;
        for (const auto & g : solo_groups)
        {
            const auto& results = _db.get_all<db::bac_result>(
                where(
                    c(&db::bac_result::dancer_id) == d.id
                    and c(&db::bac_result::group_id) == g.id));
            if (!results.empty())
                solo_group_ids.insert(g.id);
        }
        if (solo_group_ids.size() > 1)
            dancer_ids.push_back(d.id);

        std::set<int64_t> couple_group_ids;
        for (const auto & g : couple_groups)
        {
            const auto& results = _db.get_all<db::bac_result>(
                where(
                    c(&db::bac_result::dancer_id) == d.id
                    and c(&db::bac_result::group_id) == g.id));
            if (!results.empty())
                couple_group_ids.insert(g.id);
        }
        if (couple_group_ids.size() > 1)
            dancer_ids.push_back(d.id);
    }

    if (dancer_ids.empty())
        return;

    std::cout << "Potential errors in ages:\n";
    for(const auto& id : dancer_ids)
    {
        const auto& d = _db.get<db::dancer>(id);
        std::cout << fmt::format("  -- {}\n", d.name);
    }
}

} // namespace ds::validator
