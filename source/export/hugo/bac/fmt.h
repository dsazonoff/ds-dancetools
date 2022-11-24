#pragma once

namespace ds::exp::hugo
{

class fmt final
{
public:
    explicit fmt(std::ostream & os);

    fmt & yaml_header(std::string title, std::string url, std::string date, std::string image);
    fmt & h1(const std::string & text);
    fmt & h2(const std::string & text);
    fmt & h3(const std::string & text);
    fmt & h4(const std::string & text);
    fmt & h5(const std::string & text);
    fmt & couples_header();
    fmt & couple(const std::string & name1, double stars1, const std::string & name2, double stars2);
    fmt & dancers_header();
    fmt & dancer(const std::string & name, double stars);
    fmt & table_footer();
    fmt & raw(const std::string& text);
    fmt & br(size_t count = 1);

    static std::string url(const std::string & text, const std::string & url);

private:
    static std::string & fix_date(std::string & date);

private:
    std::ostream & _os;
};

} // namespace ds::exp::hugo
