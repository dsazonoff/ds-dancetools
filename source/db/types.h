#pragma once


namespace ds::db
{

// Data types

struct competition final
{
    int64_t id;
    std::string title;
    int64_t start_date;
    int64_t end_date;
    std::string url;
    double points_scale;
};

struct group final
{
    int64_t id;
    int64_t group_name_id;
    int64_t min_year;
    int64_t max_year;
    bool is_solo;
};

struct group_name final
{
    int64_t id;
    std::string name;
    std::string title;
};

struct dancer final
{
    int64_t id;
    int64_t bdsa_id;
    std::string name;
    int64_t birthday;
};

struct couple final
{
    int64_t id = {};
    std::optional<int64_t> dancer_id1;
    std::optional<int64_t> dancer_id2;
    bool is_solo = {};
};

struct result final
{
    int64_t id;
    int64_t competition_id;
    int64_t group_id;
    int64_t couple_id;
    int64_t place_start;
    int64_t place_end;
};

struct bac_result final
{
    int64_t id;
    int64_t competition_id;
    int64_t group_id;
    int64_t couple_id;
    int64_t place;
    double stars;
};


// Callbacks
namespace cb
{
using group_name = std::function<void(db::group_name)>;
using result = std::function<void(db::competition, db::group, db::group_name, std::optional<db::dancer>, std::optional<db::dancer>, db::result)>;
}

} // namespace ds::db
