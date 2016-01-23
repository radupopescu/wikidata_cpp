/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "wikidata_cpp_config.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <json.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>

using languages_t = std::array<std::string, 2>;
using site_map_t = std::unordered_map<std::string, std::string>;
using json = nlohmann::json;

struct WikidataElement
{
  std::string id;
  site_map_t sites;
};

bool parseItem(const languages_t& languages,
               const std::string& line,
               WikidataElement& elem)
{
  try {
    const auto j = json::parse(line.substr(0, line.size() - 1));
    WikidataElement el;
    el.id = j.at("id");
    const auto& s = j.at("sitelinks");
    for (const auto& l : languages) {
      el.sites[l] = s.at(l + "wiki").at("title");
    }
    elem = el;
    return true;
  } catch (std::exception& e) {
    return false;
  }
}

bool hasSameTitles(const WikidataElement& elem)
{
  const auto& title = elem.sites.begin()->second;
  return std::all_of(elem.sites.begin(), elem.sites.end(),
                     [title](const site_map_t::value_type& p)
                     {
                       return title == p.second;
                     });
}

void logResult(int n, const std::string& id)
{
  std::cout << "Processing element " << n << ":" << id << std::endl;
}

void printResults(int same, int different, double time)
{
  std::cout << "Number of items with the same titles: " << same << std::endl
            << "Number of items with different titles: " << different << std::endl
            << "Ratios: " << same * 1.0 / (same + different) << " / "
            << different * 1.0 / (same + different) << std::endl
            << "Time: " << time << "s" << std::endl;
}

int main(int argc, char** argv)
{
  if (argc != 4) {
    std::cout << "Usage: ./wikidata_cpp_main <WIKIDUMP_JSON_FILE> <LANGUAGE1> <LANGUAGE2>"
              << std::endl;
    return 1;
  }

  const std::string fileName(argv[1]);
  const languages_t languages = {argv[2], argv[3]};

  const auto t0 = std::chrono::system_clock::now();

  std::ifstream file;
  file.exceptions(std::ios::failbit | std::ios::badbit);
  file.open(fileName, std::ios_base::in | std::ios_base::binary);

  boost::iostreams::filtering_stream<boost::iostreams::input> decompressor;
  decompressor.push(boost::iostreams::gzip_decompressor());
  decompressor.push(file);

  int count = 0;
  int same = 0;
  int different = 0;
  std::string l;
  getline(decompressor, l);
  for(std::string line; getline(decompressor, line);) {
    if (line[0] != ']') {
      WikidataElement elem;
      const bool ok = parseItem(languages, line, elem);
      if (ok) {
        if (count % 1000 == 0) {
          logResult(count, elem.id);
        }
        if (hasSameTitles(elem)) {
          ++same;
        } else {
          ++different;
        }
      }
    }
    ++count;
  }

  const auto t1 = std::chrono::system_clock::now();
  const std::chrono::duration<double> dt = t1 - t0;

  printResults(same, different, dt.count());

  return 0;
}
