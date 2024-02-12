// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

struct data
{
    int place1;
    int place2;
    int number;
    std::string n1;
    std::string n2;
    std::string city;
};

std::vector<data> parse(std::istream & is)
{
    std::vector<data> out;

    const auto swap_words = [](std::string& s)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, s, boost::is_any_of(" "));
        assert(tokens.size() == 2);
        s = fmt::format("{} {}", tokens[1], tokens[0]);
    };

    data d = {};
    for (std::string line; !!std::getline(is, line);)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of("\t,"));

        if (tokens.size() == 9)
        {
            d = {};

            {
                std::vector<std::string> places;
                boost::split(places, tokens[0], boost::is_any_of("-"));
                d.place1 = boost::lexical_cast<int>(places[0]);
                if (places.size() == 2)
                    d.place2 = boost::lexical_cast<int>(places[1]);
            }
            d.number = boost::lexical_cast<int>(tokens[1]);
            d.n1 = tokens[3];
            swap_words(d.n1);
            {
                std::vector<std::string> locations;
                boost::algorithm::split_regex(locations, tokens[6], boost::regex(" / "));
                d.city = *locations.rbegin();
            }

            continue;
        }
        if (tokens.size() == 8)
        {
            d.n2 = tokens[3];
            swap_words(d.n2);
            out.push_back(std::move(d));
            d = {};
            continue;
        }
    }

    return out;
}

void save(std::ostream & os, const std::vector<data> & data)
{
    constexpr const auto noname = "- -";

    for (const auto & d : data)
    {
        os << d.place1 << ".";
        if (d.place2 > 0)
            os << "- " << d.place2 << ".";
        os << "\t" << d.number << "\t";
        os << (!d.n1.empty() ? d.n1 : noname);
        os << " / ";
        os << (!d.n2.empty() ? d.n2 : noname);
        os << "\t" << d.city << " / No club / No coach\n";
    }
}

int main()
{
    try
    {
        std::ranges::for_each(
            fs::directory_iterator{"."},
            [](const fs::directory_entry & entry)
            {
                const auto & path = entry.path();
                if (path.extension() != ".txt" || !entry.is_regular_file())
                    return;
                const auto & name = path.filename().string();

                std::ifstream is{path};
                if (!is.is_open())
                    throw std::logic_error{fmt::format("Could not read file: {}", path.generic_string())};
                const auto data = parse(is);
                is.close();

                std::ofstream os{path};
                if (!os.is_open())
                    throw std::logic_error{fmt::format("Could not write file: {}", path.generic_string())};
                save(os, data);
                os.close();
            });
    }
    catch (const std::exception & ex)
    {
        std::cerr << "Exception!\n"
                  << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!\n"
                  << std::endl;
    }

    return 0;
}
