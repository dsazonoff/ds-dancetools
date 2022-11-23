// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "db/db.h"
#include "db/populate/manifest.h"
#include "db/populate/ranking.h"
#include "db/calculate/bac/bac.h"
#include "parser/manifest/manifest_parser.h"
#include "parser/text/ranking_parser.h"
#include "export/hugo/bac/hugo.h"


int main()
{
    try
    {
        using namespace ds;

        auto db = std::make_shared<db::db>("build/db.sqlite");
        db::manifest manifest{db};
        db::ranking ranking{db};

        {
            parser::manifest_parser p;
            p.set_root_dir("data/input/text/become-a-champion");
            p.set_group_callback(manifest.callback());
            p.parse();
        }

        {
            parser::ranking_parser p;
            p.set_root_dir("data/input/text/become-a-champion");
            p.set_callback(ranking.callback());
            p.parse();
        }

        {
            db::bac b{db};
            b.evaluate(20220101, 20221231);
        }

        {
            exp::hugo::hugo h{db};
            h.set_output_dir("../sportdance-by/content/pages/db");
            h.set_suffix("become-a-champion");
            h.set_manifest("data/input/text/become-a-champion/hugo-2022.json");
            h.export_all(20220000, 20230000);
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
