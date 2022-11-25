#pragma once

#include "db/base_logic.h"


namespace ds::exp::hugo
{

class hugo final
    : private db::base_logic
{
private:
public:
    explicit hugo(const std::shared_ptr<db::db> & db);

    void set_output_dir(fs::path dir);
    void set_suffix(std::string suffix);
    void set_manifest(const fs::path & path);
    void export_all(int64_t start_date, int64_t end_date);

private:
    void export_custom(
        const fs::path & path,
        int64_t start_date,
        int64_t end_date,
        const std::string & title,
        const std::string & url,
        const std::string & header,
        const std::string & extra_header,
        bool only_passed_dancers,
        bool print_points);
    void export_passed(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & extra_header);
    void export_full_list(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & url);
    void export_competition(const fs::path & path, const std::string & url, const db::competition& comp);

private:
    fs::path _output;
    std::string _suffix;

    double _minimum_points = 0.0;
    std::string _title;
    std::string _root_url;
    std::string _banner;
    std::string _rules;
    bool _export_all = false;
    std::set<std::string> _solo_groups;
    std::set<std::string> _couple_groups;

private:
    static std::string get_surname_key(const std::string & name1, const std::string & name2 = {});
};


} // namespace ds::exp::hugo
