/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#pragma once

#include <chrono>

namespace utils {

using time_point_t = std::chrono::time_point<std::chrono::system_clock>;

time_point_t tick();

double tock(const time_point_t& t1,
            const time_point_t& t0);

}


