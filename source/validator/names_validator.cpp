// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "names_validator.h"

namespace
{

template<typename T>
size_t distance(const T & s1, const T & s2)
{
    const size_t m = s1.size();
    const size_t n = s2.size();
    if (m == 0)
        return n;
    if (n == 0)
        return m;
    std::vector<size_t> costs(n + 1);
    std::iota(costs.begin(), costs.end(), 0);
    size_t i = 0;
    for (auto c1 : s1)
    {
        costs[0] = i + 1;
        size_t corner = i;
        size_t j = 0;
        for (auto c2 : s2)
        {
            size_t upper = costs[j + 1];
            costs[j + 1] = (c1 == c2)
                ? corner
                : 1 + std::min(std::min(upper, corner), costs[j]);
            corner = upper;
            ++j;
        }
        ++i;
    }
    return costs[n];
};
} // namespace


namespace ds::validator
{

names_validator::names_validator(const std::shared_ptr<db::db> & db)
    : base_logic{db}
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

    for (const auto & d : _db.iterate<db::dancer>())
    {
        std::vector<std::string> words;
        words.reserve(2);
        boost::split(words, d.name, boost::is_any_of(" "));
        assert(words.size() == 2);
        _names.emplace(d.name, std::make_tuple(conv.from_bytes(words[0]), conv.from_bytes(words[1])));
    }
}

void names_validator::validate()
{
    std::set<std::string> err;
    for (auto l_it = _names.begin(); l_it != _names.end(); ++l_it)
        for (auto r_it = l_it; r_it != _names.end(); ++r_it)
            if (l_it != r_it)
            {
                const auto & l_full = l_it->first;
                const auto & l_name = std::get<1>(l_it->second);
                const auto & l_surname = std::get<0>(l_it->second);
                const auto & r_full = r_it->first;
                const auto & r_name = std::get<1>(r_it->second);
                const auto & r_surname = std::get<0>(r_it->second);

                auto is_suspicious = false;
                for (auto check :
                    {
                        &names_validator::levenstein,
                        &names_validator::swapped,
                    })
                    is_suspicious |= std::invoke(check, this, l_full, l_name, l_surname, r_full, r_name, r_surname);

                if (is_suspicious)
                {
                    err.insert(l_full);
                    err.insert(r_full);
                }
            }
    if (!err.empty())
    {
        std::cout << "Potential errors in names:\n";
        for(const auto& n : err)
            std::cout << fmt::format("  -- {}\n", n );
    }
}

bool names_validator::levenstein(const std::string & l_full, const std::wstring & l_name, const std::wstring & l_surname, const std::string & r_full, const std::wstring & r_name, const std::wstring & r_surname)
{
    (void)l_full;
    (void)r_full;
    const auto is_only_last_diff = [](const std::wstring & l, const std::wstring & r)
    {
        const auto diff = std::max(l.size(), r.size()) - std::min(l.size(), r.size());
        if (diff > 1)
            return false;
        if (l[0] != r[0])
            return false;
        const auto d = distance(l, r);
        return d == 1 && *l.rbegin() != *r.rbegin();
    };

    const auto d_name = distance(l_name, r_name);
    const auto d_surname = distance(l_surname, r_surname);

    auto result = false;
    // Skip names like Irina|Arina, Yan|Yana
    if (d_name == 1)
    {
        const auto only_first = (l_name.length() == r_name.length()) && (l_name[0] != r_name[0]);
        const auto only_last = is_only_last_diff(l_name, r_name);
        result |= !(only_first || only_last) && d_surname <= 1;
    }
    result |= (d_surname == 1) && !is_only_last_diff(l_surname, r_surname) && d_name <= 1;

    if (result)
        return true;

    return result;
}

bool names_validator::swapped(const std::string & l_full, const std::wstring & l_name, const std::wstring & l_surname, const std::string & r_full, const std::wstring & r_name, const std::wstring & r_surname)
{
    (void)l_full;
    (void)r_full;
    const auto d1 = distance(l_name, r_surname);
    const auto d2 = distance(r_name, l_surname);
    const auto result = (d1 <= 1) && (d2 <= 1);

    return result;
}

} // namespace ds::validator
