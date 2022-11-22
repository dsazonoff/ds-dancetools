// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "manifest.h"


namespace ds::db
{

using namespace sqlite_orm;

manifest::manifest(const std::shared_ptr<db> & db)
    : base_logic{db}
{
}

void manifest::add_group(group_name name)
{
    const auto & groups = _db.get_all<group_name>(
        where(
            c(&group_name::name) == name.name
            and c(&group_name::title) == name.title));
    get_or_insert(groups, name);
}

cb::group_name manifest::callback()
{
    return [this](group_name group_name) mutable
    {
        add_group(std::move(group_name));
    };
}

} // namespace ds::db
