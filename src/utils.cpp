/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include "utils.h"

namespace utils {

time_point_t tick()
{
  return std::chrono::system_clock::now();
}

double tock(const time_point_t& t1,
            const time_point_t& t0)
{
  const std::chrono::duration<double> dt = t1 - t0;

  return dt.count();
}

}
