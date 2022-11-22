// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "bac.h"

namespace ds::db
{

bac::bac(const std::shared_ptr<db> & db)
    : base_logic{db}
{
}

void bac::evaluate(int64_t start_date, int64_t end_date)
{
}

} // namespace ds::db
