#pragma once


namespace ds::db
{

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
    std::string id_name;
    int64_t min_year;
    int64_t max_year;
    bool is_solo;
};

struct couple final
{
    int64_t id;
    int64_t bdsa_id1;
    std::string name1;
    std::string surname1;
    int64_t bdsa_id2;
    std::string name2;
    std::string surname2;
    bool is_solo;
};

struct result final
{
    int64_t id;
    int64_t competition_id;
    int64_t group_id;
    int64_t place_start;
    int64_t place_end;
};

}
