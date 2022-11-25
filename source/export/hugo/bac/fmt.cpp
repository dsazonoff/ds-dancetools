// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "fmt.h"


namespace ds::exp::hugo
{

fmt::fmt(std::ostream & os)
    : _os{os}
{
}
fmt & fmt::yaml_header(std::string title, std::string url, std::string date, std::string image)
{
    _os << std::format(
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

std::string & fmt::fix_date(std::string & date)
{
    if (date.empty())
    {
        const auto now = std::chrono::system_clock::now();
        date = std::format("{:%Y}-{:%m}-{:%d}T00:00:00+00:00", now, now, now);
    }

    return date;
}

fmt & fmt::h1(const std::string & text)
{
    _os << "# " << text << "\n\n";
    return *this;
}

fmt & fmt::h2(const std::string & text)
{
    _os << "## " << text << "\n\n";
    return *this;
}

fmt & fmt::h3(const std::string & text)
{
    _os << "### " << text << "\n\n";
    return *this;
}

fmt & fmt::h4(const std::string & text)
{
    _os << "#### " << text << "\n\n";
    return *this;
}

fmt & fmt::h5(const std::string & text)
{
    _os << "##### " << text << "\n\n";
    return *this;
}

std::string fmt::url(const std::string & text, const std::string & url)
{
    return std::format("[{}]({})", text, url);
}

fmt & fmt::couples_header(bool print_stars)
{
    if (print_stars)
        _os << "| Партнёр | &nbsp;&nbsp;&nbsp; | Звёзды | &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | Партнёрша | &nbsp;&nbsp;&nbsp; | Звёзды |\n";
    else
        _os << "| Партнёр | &nbsp;&nbsp;&nbsp; | &nbsp; | &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | Партнёрша | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    _os << "|:--|-|:--:|-|:--|-|:--:|\n";

    return *this;
}

fmt & fmt::table_footer()
{
    _os << "\n";
    return *this;
}

fmt & fmt::couple(const std::string & name1, double stars1, const std::string & name2, double stars2)
{
    _os << std::format("|{}|&nbsp;|{}| |{}|&nbsp;|{}|\n", name1, stars1, name2, stars2);
    return *this;
}

fmt & fmt::couple(const std::string & name1, const std::string & name2)
{
    _os << std::format("|{}|&nbsp;| | |{}|&nbsp;| |\n", name1, name2);

    return *this;
}

fmt & fmt::dancers_header(bool print_stars)
{
    if (print_stars)
        _os << "| Фамилия Имя | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    else
        _os << "| Фамилия Имя | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    _os << "|:--|-|:--:|\n";

    return *this;
}

fmt & fmt::dancer(const std::string & name, double stars)
{
    _os << std::format("|{}|&nbsp;|{}|\n", name, stars);
    return *this;
}

fmt & fmt::dancer(const std::string & name)
{
    _os << std::format("|{}|&nbsp;| |\n", name);
    return *this;
}

fmt & fmt::raw(const std::string & text)
{
    _os << text;
    return *this;
}

fmt & fmt::br(size_t count)
{
    for (size_t i = 0; i < count; ++i)
        _os << '\n';
    return *this;
}


} // namespace ds::exp::hugo
