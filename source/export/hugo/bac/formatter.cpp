// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "formatter.h"


namespace ds::exp::hugo
{

formatter::formatter(std::ostream & os)
    : _os{os}
{
}
formatter & formatter::yaml_header(std::string title, std::string url, std::string date, std::string image)
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

std::string & formatter::fix_date(std::string & date)
{
    if (date.empty())
    {
        const auto now = std::chrono::system_clock::now();
        date = fmt::format("{:%Y-:%m-:%d}T00:00:00+00:00", now);
    }

    return date;
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

formatter & formatter::couples_header(bool print_stars)
{
    if (print_stars)
        _os << "| Партнёр | &nbsp;&nbsp;&nbsp; | Звёзды | &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | Партнёрша | &nbsp;&nbsp;&nbsp; | Звёзды |\n";
    else
        _os << "| Партнёр | &nbsp;&nbsp;&nbsp; | &nbsp; | &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | Партнёрша | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    _os << "|:--|-|:--:|-|:--|-|:--:|\n";

    return *this;
}

formatter & formatter::table_footer()
{
    _os << "\n";
    return *this;
}

formatter & formatter::couple(const std::string & name1, double stars1, const std::string & name2, double stars2)
{
    _os << fmt::format("|{}|&nbsp;|{}| |{}|&nbsp;|{}|\n", name1, stars1, name2, stars2);
    return *this;
}

formatter & formatter::couple(const std::string & name1, const std::string & name2)
{
    _os << fmt::format("|{}|&nbsp;| | |{}|&nbsp;| |\n", name1, name2);

    return *this;
}

formatter & formatter::dancers_header(bool print_stars)
{
    if (print_stars)
        _os << "| Фамилия Имя | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    else
        _os << "| Фамилия Имя | &nbsp;&nbsp;&nbsp; | &nbsp; |\n";
    _os << "|:--|-|:--:|\n";

    return *this;
}

formatter & formatter::dancer(const std::string & name, double stars)
{
    _os << fmt::format("|{}|&nbsp;|{}|\n", name, stars);
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


} // namespace ds::exp::hugo
