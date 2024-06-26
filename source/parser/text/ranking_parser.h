#pragma once

#include "types/types.h"


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
        std::map<std::string, std::vector<db::bac_group_split>> group_results;
        db::group group;
    };

public:
    ranking_parser();

    void set_root_dir(fs::path dir);
    void set_callback(db::cb::result result_callback, db::cb::group_split split_callback);
    void parse();

private:
    void parse_results_dir(const fs::path & results_dir);
    db::competition parse_competition(const json::value & manifest) const;
    std::map<std::string, db::group> parse_groups(const json::value & manifest) const;
    std::map<std::string, std::vector<db::bac_group_split>> parse_group_results(const json::value & manifest) const;
    void parse_results_file(const fs::path & path);
    std::tuple<std::optional<db::dancer>, std::optional<db::dancer>, db::result, db::city> parse_line(const std::string & line) const;

private:
    fs::path _root = ".";
    context _ctx;
    db::cb::result _result_callback;
    db::cb::group_split _split_callback;
};

} // namespace ds::parser
