#pragma once

#include "db/types.h"


namespace ds::parser
{

class ranking_parser final
{
private:
    struct context final
    {
        json::value manifest;
        fs::path working_dir;
        fs::path file;
        db::competition competition;
        std::map<std::string, db::group> groups;
        db::group group;
    };

public:
    using result_cb = std::function<void(db::competition, const db::group, db::couple, db::result)>;

public:
    ranking_parser();

    void set_root_dir(fs::path dir);
    void set_callback(result_cb callback);
    void parse();

private:
    void parse_results_dir(const fs::path & results_dir);
    db::competition parse_competition(const json::value & manifest) const;
    std::map<std::string, db::group> parse_groups(const json::value & manifest) const;
    void parse_results_file(const fs::path & path);
    std::tuple<db::couple, db::result> parse_line(const std::string& line) const;

private:
    fs::path _root = ".";
    context _ctx;
    result_cb _result_callback;
};

} // namespace ds::parser
