/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "data.h"

#include <json.hpp>

using json = nlohmann::json;

namespace data {

bool parseItem(const Languages& languages, const std::string& line, WikidataElement& elem)
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

}
