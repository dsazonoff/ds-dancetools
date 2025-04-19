#pragma once

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/exception/all.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/uuid/name_generator_sha1.hpp>

namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace json = boost::json;
