// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "db/calculate/bac/bac.h"
#include "db/calculate/bac/override_bac.h"
#include "db/db.h"
#include "db/populate/manifest.h"
#include "db/populate/ranking.h"
#include "export/hugo/bac/hugo.h"
#include "parser/manifest/manifest_parser.h"
#include "parser/text/ranking_parser.h"
#include "validator/names_validator.h"
#include "validator/age_validator.h"


namespace
{
constexpr const auto s_results_path = "data";
constexpr const auto s_output_path = "../sportdance-by/content/pages/db";
} // namespace


int main()
{
    try
    {
        using namespace ds;

        std::cout << fmt::format("Running dancetools version: {}\n", DS_VERSION);

        const boost::locale::generator generator;
        const std::locale loc = generator("ru_RU.UTF-8");
        std::locale::global(loc);

        const auto mgr = boost::locale::localization_backend_manager::global();
        const auto backends = mgr.get_all_backends();
        ds_assert(std::ranges::find(backends, "icu") != backends.end());

        const auto detect_year = []()
        {
            std::set<std::string> dirs;
            for (const auto & entry : fs::directory_iterator{s_results_path, fs::directory_options::skip_permission_denied})
            {
                if (entry.is_directory())
                    dirs.insert(entry.path().filename().string());
            }
            if (dirs.empty())
                throw std::logic_error{fmt::format("Could not find year data directory in: {}", s_results_path)};
            auto year = *dirs.rbegin();
            if (year.size() != 4)
                throw std::logic_error{fmt::format("Could not find year data directory in: {}", s_results_path)};
            return year;
        };

        const auto year = detect_year();
        int year_num = 0;
        std::from_chars(year.data(), year.data() + year.size(), year_num);
        if (year_num == 0)
            throw std::logic_error{fmt::format("Could not find year data directory in: {}", s_results_path)};

        const auto results_path = fs::path{s_results_path} / year;
        const auto output_path = fs::path{s_output_path} / year;
        const auto range_begin = (year_num + 0) * 10000;
        const auto range_end = (year_num + 1) * 10000;

        // auto db = std::make_shared<db::db>("build/db.sqlite");
        auto db = std::make_shared<db::db>(":memory:");
        db::manifest manifest{db};
        db::ranking ranking{db};

        {
            parser::manifest_parser p;
            p.set_root_dir(results_path);
            p.set_group_callback(manifest.callback());
            p.parse();
        }

        {
            parser::ranking_parser p;
            p.set_root_dir(results_path);
            p.set_callback(ranking.result_callback(), ranking.split_callback());
            p.parse();
        }

        {
            const auto override_path = fs::path{results_path} / "override.json";
            db::bac b{db};
            b.evaluate(range_begin, range_end, override_path);
        }

        {
            fs::remove_all(output_path);

            exp::hugo::hugo h{db};
            h.set_output_dir(output_path);
            h.set_suffix("become-a-champion");
            const auto manifest_path = fs::path{results_path} / "hugo.json";
            h.set_manifest(manifest_path);
            h.export_all(range_begin, range_end);
        }

        {
            validator::names_validator names{db};
            names.validate();

            validator::age_validator age{db};
            age.validate();
        }
    }
    catch (const std::exception & ex)
    {
        std::cerr << "Exception!\n"
                  << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!\n"
                  << std::endl;
    }

    return 0;
}
