/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "data.h"

#include <rapidjson/Document.h>

#include <iostream>

namespace rj = rapidjson;

namespace data {

bool parseItem(const Languages& languages, const std::string& line, WikidataElement& elem)
{
  rj::Document j;
  j.Parse(line.substr(0, line.size() - 1).c_str());
  if (! j.HasParseError()) {
    if (j.HasMember("id")) {
      elem.id = j["id"].GetString();
      if (j.HasMember("sitelinks")) {
        const auto& s = j["sitelinks"].GetObject();
        for (const auto& l : languages) {
          const auto sitelink = l + "wiki";
          if (s.HasMember(sitelink.c_str())) {
            if (s[sitelink.c_str()].HasMember("title")) {
              elem.sites[l] = s[sitelink.c_str()]["title"].GetString();
            }
          }
        }
        return elem.sites.size() == languages.size();
      }
    }
  }

  return false;
}

}
