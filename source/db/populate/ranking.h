#pragma once

#include "db/base_logic.h"

namespace ds::db
{

class ranking final
    : private base_logic
{
public:
    explicit ranking(const std::shared_ptr<db> & db);

    cb::result callback();

private:
    void add_city(city & c);
    void add_competition(competition & cp);
    void add_group(group & g);
    void add_dancer(std::optional<dancer> & d);
    void add_couple(couple & cpl);
    void add_result(result & r);
};


} // namespace ds::db
