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
#include "pipeline.h"
#include "processing.h"
#include "utils.h"

#include <zmqpp/zmqpp.hpp>

#include <iostream>

int main(int argc, char** argv)
{
  if (argc < 4) {
    std::cout << "Usage: ./wikidata_cpp_main <WIKIDUMP_JSON_FILE> <NUM_WORKERS> "
                 "<LANGUAGE 1> <LANGUAGE 2> ... <LANGUAGE N>"
              << std::endl;
    return 1;
  }

  const std::string fileName(argv[1]);
  const int numWorkers = std::atoi(argv[2]);

  data::Languages languages;
  for (auto i = 3; i < argc; ++i) {
    languages.push_back(argv[i]);
  }

  const auto t0 = utils::tick();

  zmqpp::context context;
  zmqpp::reactor mainReactor;

  // Collector
  const zmqpp::endpoint_t collectorAddress("inproc://collector");
  zmqpp::socket collector(context, zmqpp::socket_type::pull);
  collector.set(zmqpp::socket_option::linger, 1000);
  collector.bind(collectorAddress);

  int numLinesRead = 0;
  int numLinesProcessed = 0;
  int same = 0;
  int different = 0;
  mainReactor.add(collector, [&same, &different, &numLinesRead, &numLinesProcessed, &collector]() {
    zmqpp::message msg;
    collector.receive(msg);
    std::string msgText;
    msg >> msgText;
    if (msgText.compare("same") == 0) {
      ++same;
    } else if (msgText.compare("diff") == 0) {
      ++different;
    }
    // Message is "inv" - just count line
    ++numLinesProcessed;
  });

  // Monitor
  const zmqpp::endpoint_t killAddress("inproc://kill");
  zmqpp::socket killBroadcast(context, zmqpp::socket_type::pub);
  killBroadcast.bind(killAddress);

  const zmqpp::endpoint_t monitorAddress("inproc://monitor");
  zmqpp::socket monitor(context, zmqpp::socket_type::pull);
  monitor.set(zmqpp::socket_option::linger, 1000);
  monitor.bind(monitorAddress);

  bool streamingComplete = false;
  mainReactor.add(monitor, [&numLinesRead, &streamingComplete, &monitor]() {
    zmqpp::message msg;
    monitor.receive(msg);
    msg >> numLinesRead;
    streamingComplete = true;
  });

  // Logger
  const int loggingInterval = 1000;
  const auto loggerAddress("inproc://logger");
  auto logger = std::thread([&context, loggerAddress, killAddress, loggingInterval]() {
    pipeline::loggerTask(context, loggerAddress, killAddress, loggingInterval);
  });

  // Workers
  const auto workerAddress("inproc://worker");
  std::vector<std::thread> workers;
  for (int i = 0; i < numWorkers; ++i) {
    workers.emplace_back([&context, workerAddress, collectorAddress, killAddress, loggerAddress, languages]() {
      pipeline::workerTask(context, workerAddress, collectorAddress, killAddress, loggerAddress, languages);
    });
  }

  // Streamer
  auto streamer = std::thread([&context, workerAddress, monitorAddress, fileName]() {
    pipeline::streamerTask(context, workerAddress, monitorAddress, fileName);
  });

  // Reactor loop
  while (!streamingComplete || (numLinesProcessed < numLinesRead)) {
    mainReactor.poll(10);
  }

  // Send kill signal
  zmqpp::message killMsg;
  killMsg << "Notifications"
          << "Kill!!!";
  killBroadcast.send(killMsg);

  // Cleanup
  streamer.join();
  for (auto& w : workers) {
    w.join();
  }
  logger.join();

  const auto t1 = utils::tick();
  io::printResults(same, different, utils::tock(t1, t0));

  return 0;
}
