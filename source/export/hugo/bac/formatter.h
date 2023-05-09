#pragma once

namespace ds::exp::hugo
{

class formatter final
{
public:
    explicit formatter(std::ostream & os);

    formatter & yaml_header(const std::string & title, const std::string & url, const std::string & date, const std::string & image);
    formatter & h1(const std::string & text);
    formatter & h2(const std::string & text);
    formatter & h3(const std::string & text);
    formatter & h4(const std::string & text);
    formatter & h5(const std::string & text);
    formatter & couples_header(bool print_points = true);
    formatter & couple(const std::string & name1, double points1, const std::string & name2, double points2);
    formatter & couple(const std::string & name1, const std::string & name2);
    formatter & couple(const std::string & name1, const std::string & name2, double points);
    formatter & dancers_header(bool print_points = true);
    formatter & dancer(const std::string & name, double points);
    formatter & dancer(const std::string & name);
    formatter & table_footer();
    formatter & raw(const std::string & text);
    formatter & br(size_t count = 1);
    formatter & list(const std::string & text);

    static std::string url(const std::string & text, const std::string & url);

private:
    static std::string fix_date(const std::string & date);

private:
    std::ostream & _os;
};

} // namespace ds::exp::hugo
