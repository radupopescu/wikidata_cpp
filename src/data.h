/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace data {

using Languages = std::vector<std::string>;
using SiteMap = std::unordered_map<std::string, std::string>;

struct WikidataElement
{
  std::string id;
  SiteMap sites;
};

bool parseItem(const Languages& languages, const std::string& line, WikidataElement& elem);

}
