/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "data.h"
#include "io.h"
#include "processing.h"
#include "utils.h"

#include <iostream>

int main(int argc, char** argv)
{
  if (argc < 4) {
    std::cout << "Usage: ./wikidata_cpp_main <WIKIDUMP_JSON_FILE> <LANGUAGE 1> <LANGUAGE 2> ... <LANGUAGE N>"
              << std::endl;
    return 1;
  }

  const std::string fileName(argv[1]);

  data::Languages languages;
  for (auto i = 2; i < argc; ++i) {
    languages.push_back(argv[i]);
  }

  const auto t0 = utils::tick();

  io::Streamer streamer(fileName);

  int count = 0;
  int same = 0;
  int different = 0;
  for(std::string line; streamer.getLine(line);) {
    data::WikidataElement elem;
    const bool ok = data::parseItem(languages, line, elem);
    if (ok) {
      if (count % 1000 == 0) {
        io::logResult(count, elem.id);
      }
      if (processing::hasSameTitles(elem)) {
        ++same;
      } else {
        ++different;
      }
    }
    ++count;
  }

  const auto t1 = utils::tick();
  io::printResults(same, different, utils::tock(t1, t0));

  return 0;
}
