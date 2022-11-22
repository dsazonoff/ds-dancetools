// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "base_logic.h"


namespace ds::db
{

base_logic::base_logic(const std::shared_ptr<db> & db)
    : _db{(*db)()}
    , _database{db}
{
}


} // namespace ds::db
