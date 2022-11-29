#pragma once


namespace ds::db::utils
{

template<typename C>
std::vector<int64_t> ids(C & c);


template<typename C>
std::vector<int64_t> ids(C & c)
{
    ds_assert(!c.empty());
    std::vector<int64_t> r;
    r.reserve(c.size());
    for (const auto & item : c)
        r.push_back(item.id);
    return r;
}

} // namespace ds::db::utils
