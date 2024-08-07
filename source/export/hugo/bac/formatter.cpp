// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "formatter.h"


namespace ds::exp::hugo
{

formatter::formatter(std::ostream & os)
    : _os{os}
{
}

formatter & formatter::yaml_header(const std::string & title, const std::string & url, const std::string & date, const std::string & image)
{
    _os << fmt::format(
        R"(---
title: {}
url: "{}"
date: {}
headerTransparent: false
sections:
- template: content
  align: center
  columns: 11
  image: "{}"
---
)",
        title, url, fix_date(date), image);

    return *this;
}

std::string formatter::fix_date(const std::string & date)
{
    if (!date.empty())
        return date;

    const auto now = std::chrono::system_clock::now();
    auto current_date = fmt::format("{:%Y-01-01}T00:00:00+00:00", now);

    return current_date;
}

formatter & formatter::h1(const std::string & text)
{
    _os << "# " << text << "\n";
    return *this;
}

formatter & formatter::h2(const std::string & text)
{
    _os << "## " << text << "\n";
    return *this;
}

formatter & formatter::h3(const std::string & text)
{
    _os << "### " << text << "\n";
    return *this;
}

formatter & formatter::h4(const std::string & text)
{
    _os << "#### " << text << "\n";
    return *this;
}

formatter & formatter::h5(const std::string & text)
{
    _os << "##### " << text << "\n";
    return *this;
}

std::string formatter::url(const std::string & text, const std::string & url)
{
    return fmt::format("[{}]({})", text, url);
}

formatter & formatter::couples_header(bool print_points)
{
    _os << "| Партнёр | &nbsp;&nbsp;&nbsp; | Партнёрша | &nbsp;&nbsp;&nbsp; |";

    if (print_points)
        _os << " Баллы |\n";
    else
        _os << " &nbsp; |\n";
    _os << "|:--|-|:--|-|:--:|\n";

    return *this;
}

formatter & formatter::table_footer()
{
    _os << "\n";
    return *this;
}

formatter & formatter::couple(const std::string & name1, double points1, const std::string & name2, double points2)
{
    _os << fmt::format("|{}|&nbsp;|{:.1f}| |{}|&nbsp;|{:.1f}|\n", name1, points1, name2, points2);
    return *this;
}

formatter & formatter::couple(const std::string & name1, const std::string & name2)
{
    _os << fmt::format("|{}|&nbsp;| | |{}|&nbsp;| |\n", name1, name2);

    return *this;
}

formatter & formatter::couple(const std::string & name1, const std::string & name2, double points)
{
    _os << fmt::format("|{}|&nbsp;|{}|&nbsp;|{:.1f}|\n", name1, name2, points);
    return *this;
}

formatter & formatter::dancers_header(bool print_points)
{
    if (print_points)
        _os << "| Имя Фамилия | &nbsp;&nbsp;&nbsp; | Баллы |\n";
    else
        _os << "| Имя Фамилия | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    _os << "|:--|-|:--:|\n";

    return *this;
}

formatter & formatter::dancer(const std::string & name, double points)
{
    _os << fmt::format("|{}|&nbsp;|{:.1f}|\n", name, points);
    return *this;
}

formatter & formatter::dancer(const std::string & name)
{
    _os << fmt::format("|{}|&nbsp;| |\n", name);
    return *this;
}

formatter & formatter::raw(const std::string & text)
{
    _os << text;
    return *this;
}

formatter & formatter::br(size_t count)
{
    for (size_t i = 0; i < count; ++i)
        _os << '\n';
    return *this;
}

formatter & formatter::list(const std::string & text)
{
    _os << fmt::format("- {}\n", text);
    return *this;
}

formatter & formatter::timestamp()
{
    static const auto now = std::chrono::system_clock::now();
    const auto now_time_t = std::chrono::system_clock::to_time_t(now);
    const auto utc_tm = std::gmtime(&now_time_t);
    const std::time_t utc_time_t = std::mktime(utc_tm);
    const auto utc_time_point = std::chrono::system_clock::from_time_t(utc_time_t);

    const auto current_date = fmt::format("Compiled at: {:%d-%m-%Y %H:%M} GMT", utc_time_point);
    br(2);
    _os << current_date;

    return *this;
}


} // namespace ds::exp::hugo
