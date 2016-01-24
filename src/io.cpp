/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "io.h"

#include <boost/iostreams/filter/gzip.hpp>

#include <iostream>

namespace io {

void makeStreamer(std::ifstream& file, streamer_t& streamer, const std::string& fileName)
{
  file.exceptions(std::ios::failbit | std::ios::badbit);
  file.open(fileName, std::ios_base::in | std::ios_base::binary);

  streamer.push(boost::iostreams::gzip_decompressor());
  streamer.push(file);
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

}
