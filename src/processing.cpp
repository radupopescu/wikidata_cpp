/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "processing.h"

#include <algorithm>

namespace processing {

bool hasSameTitles(const data::WikidataElement& elem)
{
  const auto& title = elem.sites.begin()->second;
  return std::all_of(elem.sites.begin(), elem.sites.end(),
                     [title](const data::site_map_t::value_type& p)
                     {
                       return title == p.second;
                     });
}

}
