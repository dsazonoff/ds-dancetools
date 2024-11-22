#pragma once

#include "db/base_logic.h"
#include "types/types.h"


namespace ds::validator
{

class age_validator final
    : public db::base_logic
{
public:
    explicit age_validator(const std::shared_ptr<db::db> & db);

    void validate();

private:
};


} // namespace ds::validator
