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


namespace
{
constexpr const auto s_results_path = "data/input/text/2023";
constexpr const auto s_output_path = "../content/pages/db/2023";
} // namespace


int main()
{
    try
    {
        using namespace ds;

        std::cout << fmt::format("Running dancetools version: {}\n", DS_VERSION);

        // auto db = std::make_shared<db::db>("build/db.sqlite");
        auto db = std::make_shared<db::db>(":memory:");
        db::manifest manifest{db};
        db::ranking ranking{db};

        {
            parser::manifest_parser p;
            p.set_root_dir(s_results_path);
            p.set_group_callback(manifest.callback());
            p.parse();
        }

        {
            parser::ranking_parser p;
            p.set_root_dir(s_results_path);
            p.set_callback(ranking.result_callback(), ranking.split_callback());
            p.parse();
        }

        {
            const auto override_path = fs::path{s_results_path} / "override-2023.json";
            db::bac b{db};
            b.evaluate(20230000, 20240000, override_path);
        }

        {
            fs::remove_all(s_output_path);

            exp::hugo::hugo h{db};
            h.set_output_dir(s_output_path);
            h.set_suffix("become-a-champion");
            const auto manifest_path = fs::path{s_results_path} / "hugo-2023.json";
            h.set_manifest(manifest_path);
            h.export_all(20230000, 20240000);
        }

        {
            validator::names_validator v{db};
            v.validate();
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
