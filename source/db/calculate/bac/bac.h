#pragma once

#include "db/base_logic.h"


namespace ds::db
{

class bac final
    : public base_logic
{
public:
    explicit bac(const std::shared_ptr<db> & db);

    void evaluate(int64_t start_date, int64_t end_date);
};

}
