#pragma once

#include "db/base_logic.h"


namespace ds::db
{

class bac final
    : public base_logic
{
private:
    struct context final
    {
        std::vector<competition> competitions;
        competition competition;
        int64_t host_city_id;
        std::vector<group> groups;
        group group;
    };

public:
    explicit bac(const std::shared_ptr<db> & db);

    void evaluate(int64_t start_date, int64_t end_date, const fs::path& override_path);

private:
    void proceed_competition(const competition & comp);
    void proceed_group(const group & gr);
    void proceed_result(const result & r, size_t place, double points);
    void update_points(int64_t start_date, int64_t end_date);

private:
    context _ctx;
    const std::shared_ptr<db> _db_ptr;
};

} // namespace ds::db
