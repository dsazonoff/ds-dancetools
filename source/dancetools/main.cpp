// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "parser/manifest/manifest_parser.h"
#include "parser/text/ranking_parser.h"

int main()
{
    try
    {
        using namespace ds;
        parser::manifest_parser p;
        p.set_root_dir("data/input/text/become-a-champion");
        p.parse();
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
