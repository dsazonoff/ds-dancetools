#pragma once

#include "db/db.h"

namespace ds::db
{

class base_logic
{
public:
    explicit base_logic(const std::shared_ptr<db> & db);

    template<typename C, typename R>
    void get_or_insert(const C & container, R & record);

protected:
    db::db_t & _db;

private:
    std::shared_ptr<db> _database;
};


template<typename C, typename R>
void base_logic::get_or_insert(const C & container, R & record)
{
    ds_assert(container.size() <= 1);
    record.id = container.empty()
        ? _db.insert(record)
        : container[0].id;
}

} // namespace ds::db
