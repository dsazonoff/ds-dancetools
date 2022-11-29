// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "db.h"

namespace ds::db
{

db::db(const std::string & path)
    : _db{make_db(path)}
{
    _db.sync_schema();
}

db::db_t & db::operator()()
{
    return _db;
}

} // namespace ds::db
