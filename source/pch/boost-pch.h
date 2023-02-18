#pragma once

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/range.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/detail/sha1.hpp>

namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace json = boost::json;
