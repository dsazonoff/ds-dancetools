#pragma once


#include "db/types.h"

namespace ds::parser
{

class manifest_parser final
{
public:
    using group_cb = std::function<void(db::group_name)>;

public:
    manifest_parser();

    void set_root_dir(fs::path dir);
    void set_group_callback(group_cb callback);
    void parse();

private:
    fs::path _root = ".";
    group_cb _group_callback;
};

} // namespace ds::parser
