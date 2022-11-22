// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "db/db.h"
#include "db/populate/manifest.h"
#include "db/populate/ranking.h"
#include "parser/manifest/manifest_parser.h"
#include "parser/text/ranking_parser.h"

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
