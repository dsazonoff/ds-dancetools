#pragma once

#include "base_logic.h"

namespace ds::db
{

class ranking final
    : private base_logic
{
public:
    explicit ranking(const std::shared_ptr<db> & db);

    cb::result callback();

private:
    void add_competition(competition& cp);
    void add_group(group& g);
    void add_couple(couple& cpl);
    void add_result(result& r);
};


} // namespace ds::db
