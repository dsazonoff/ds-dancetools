#pragma once

#include "db/base_logic.h"

namespace ds::db
{

class manifest final
    : private base_logic
{
public:
    explicit manifest(const std::shared_ptr<db> & db);

    cb::group_name callback();

private:
    void add_group(group_name name);
};

} // namespace ds::db
