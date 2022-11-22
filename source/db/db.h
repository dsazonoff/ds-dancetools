#pragma once

#include "types.h"

namespace ds::db
{

inline auto make_db(const std::string & path)
{
    using namespace sqlite_orm;

    // clang-format off
    auto db = make_storage(path,
        make_table("group_names",
            make_column("id",       &group_name::id,                autoincrement(), primary_key()),
            make_column("name",     &group_name::name,              unique()),
            make_column("title",    &group_name::title,             unique())
            ),

        make_table("groups",
            make_column("id",               &group::id,             autoincrement(), primary_key()),
            make_column("group_name_id",    &group::group_name_id),
            make_column("min_year",         &group::min_year),
            make_column("max_year",         &group::max_year),
            make_column("is_solo",          &group::is_solo),

            foreign_key(&group::group_name_id)          .references(&group_name::id)
            ),

        make_table("competitions",
            make_column("id",               &competition::id,       autoincrement(), primary_key()),
            make_column("title",            &competition::title),
            make_column("start_date",       &competition::start_date),
            make_column("end_date",         &competition::end_date),
            make_column("url",              &competition::url),
            make_column("points_scale",     &competition::points_scale)
            ),

        make_table("dancers",
            make_column("id",               &dancer::id,            autoincrement(), primary_key()),
            make_column("bdsa_id",          &dancer::bdsa_id),
            make_column("name",             &dancer::name),
            make_column("birthday",         &dancer::birthday)
            ),

        make_table("couples",
            make_column("id",               &couple::id,            autoincrement(), primary_key()),
            make_column("dancer_id1",       &couple::dancer_id1),
            make_column("dancer_id2",       &couple::dancer_id2),
            make_column("is_solo",          &couple::is_solo),

            foreign_key(&couple::dancer_id1)            .references(&dancer::id),
            foreign_key(&couple::dancer_id2)            .references(&dancer::id)
            ),

        make_table("results",
            make_column("id",               &result::id,             autoincrement(), primary_key()),
            make_column("competition_id",   &result::competition_id),
            make_column("group_id",         &result::group_id),
            make_column("couple_id",        &result::couple_id),
            make_column("place_start",      &result::place_start),
            make_column("place_end",        &result::place_end),

            foreign_key(&result::competition_id)        .references(&competition::id),
            foreign_key(&result::group_id)              .references(&group::id),
            foreign_key(&result::couple_id)             .references(&couple::id)
            ),

        make_table("bac_results",
            make_column("id",               &bac_result::id,             autoincrement(), primary_key()),
            make_column("competition_id",   &bac_result::competition_id),
            make_column("group_id",         &bac_result::group_id),
            make_column("couple_id",        &bac_result::couple_id),
            make_column("place",            &bac_result::place),
            make_column("stars",            &bac_result::stars),

            foreign_key(&bac_result::competition_id)    .references(&competition::id),
            foreign_key(&bac_result::group_id)          .references(&group::id),
            foreign_key(&bac_result::couple_id)         .references(&couple::id)
            )

    );
    // clang-format on

    return db;
}

class db final
{
public:
    using db_t = std::invoke_result_t<decltype(make_db), const std::string &>;

public:
    explicit db(const std::string & path);

    db_t & operator()();

private:
    db_t _db;
};

} // namespace ds::db
