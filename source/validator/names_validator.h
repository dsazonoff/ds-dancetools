#pragma once

#include "db/base_logic.h"
#include "types/types.h"


namespace ds::validator
{

class names_validator final
    : public db::base_logic
{
public:
    explicit names_validator(const std::shared_ptr<db::db> & db);

    void validate();

private:
    bool levenstein(const std::string& l_full, const std::wstring& l_name, const std::wstring& l_surname, const std::string& r_full, const std::wstring& r_name, const std::wstring& r_surname);
    bool swapped(const std::string& l_full, const std::wstring& l_name, const std::wstring& l_surname, const std::string& r_full, const std::wstring& r_name, const std::wstring& r_surname);
    bool name_check_01(const std::string& l_full, const std::wstring& l_name, const std::wstring& l_surname, const std::string& r_full, const std::wstring& r_name, const std::wstring& r_surname);

    std::map<std::string, std::tuple<std::wstring, std::wstring>> _names;
};


} // namespace ds::validator
