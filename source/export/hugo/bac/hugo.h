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
        bool print_points,
        bool sort_points);
    void export_full_list(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & extra_header);
    void export_ranking(const fs::path & path, int64_t start_date, int64_t end_date, const std::string & url);
    void export_competition(const fs::path & path, const std::string & url, const db::competition & comp);
    void export_all_dancers(const fs::path & path, const std::string & url, int64_t start_date, int64_t end_date);
    void export_dancer(const fs::path & path, const std::string & url, const db::dancer & dancer, const std::vector<db::competition> & competitions, int64_t start_date, int64_t end_date);

private:
    fs::path _output;
    std::string _suffix;

    std::string _title;
    std::string _root_url;
    std::string _banner;
    std::string _rules;
    bool _export_all = false;
    bool _export_details = false;
    std::set<std::string> _solo_groups;
    std::set<std::string> _couple_groups;

private:
    static std::string get_surname_key(const std::string & name1, const std::string & name2 = {}, const std::string& sep = {});
    static std::string get_points_key(double points, const std::string & name1, const std::string & name2 = {});
};


} // namespace ds::exp::hugo
