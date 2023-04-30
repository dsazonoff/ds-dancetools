#pragma once


#include "db/base_logic.h"
#include "types/types.h"


namespace ds::db
{

class override_bac final
    : public base_logic
{
public:
    enum class action
    {
        move,
        remove,
    };

    struct move_dancer final
    {
        std::string name;
        std::string from_group;
        std::string to_group;
    };

    struct remove_dancer final
    {
        std::string name;
        std::string from_group;
        int64_t start_date;
        int64_t end_date;
    };

    struct add_points final
    {
        std::string name;
        std::string group;
        int64_t start_date;
        int64_t end_date;
        double points;
    };

    struct remove_couple final
    {
        std::string male;
        std::string female;
        std::string from_group;
    };

    using data_t = std::variant<move_dancer, remove_dancer, remove_couple, add_points>;

public:
    explicit override_bac(const std::shared_ptr<db> & db);

    void set_config(const fs::path & path);
    void apply(int64_t start_date, int64_t end_date);

private:
    void on_move(const move_dancer & data, int64_t start_date, int64_t end_date);
    void on_remove(const remove_dancer & data, int64_t start_date, int64_t end_date);
    void on_add_points(const add_points & data, int64_t start_date, int64_t end_date);
    void on_remove(const remove_couple & data, int64_t start_date, int64_t end_date);

private:
    std::vector<data_t> _rules;
};

} // namespace ds::db
