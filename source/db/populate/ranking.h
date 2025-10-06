#pragma once

#include "db/base_logic.h"

namespace ds::db
{

class ranking final
    : private base_logic
{
public:
    explicit ranking(const std::shared_ptr<db> & db);

    cb::result result_callback();
    cb::group_split split_callback();

private:
    void add_city(city & c);
    void add_competition(competition & cp);
    void add_group(group & g);
    void add_dancer(std::optional<dancer> & d);
    void add_couple(couple & cpl);
    void add_result(result & r);
    void add_split(bac_group_split & s);
};


} // namespace ds::db
