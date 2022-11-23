#pragma once


#include "types/types.h"

namespace ds::parser
{

class manifest_parser final
{
public:
    manifest_parser();

    void set_root_dir(fs::path dir);
    void set_group_callback(db::cb::group_name callback);
    void parse();

private:
    fs::path _root = ".";
    db::cb::group_name _group_callback;
};

} // namespace ds::parser
