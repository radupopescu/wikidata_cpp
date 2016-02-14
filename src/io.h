/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#pragma once

#include <boost/iostreams/filtering_stream.hpp>

#include <fstream>

namespace io {

class Streamer {
public:
  Streamer(const std::string& fileName);

  bool getLine(std::string& line);

private:
  std::ifstream file_;
  boost::iostreams::filtering_stream<boost::iostreams::input> boostStreamer_;
};

void logResult(int n, const std::string& id);

void printResults(int same, int different, double time);

}
