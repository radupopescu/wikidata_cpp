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

#include <zmqpp/zmqpp.hpp>

#include <iostream>

void streamerTask(const zmqpp::context& ctx,
                  const zmqpp::endpoint_t& out,
                  const zmqpp::endpoint_t& mon,
                  const std::string& fileName)
{
  zmqpp::socket output(ctx, zmqpp::socket_type::push);
  output.set(zmqpp::socket_option::linger, 1000);
  output.bind(out);

  zmqpp::socket monitor(ctx, zmqpp::socket_type::push);
  monitor.set(zmqpp::socket_option::linger, 1000);
  monitor.connect(mon);

  int numLinesRead = 0;
  io::Streamer streamer(fileName);
  for(std::string line; streamer.getLine(line);) {
    output.send(line);
    ++numLinesRead;
  }

  zmqpp::message endMsg;
  endMsg << numLinesRead;
  monitor.send(endMsg);
}

void workerTask(const zmqpp::context& ctx,
                const zmqpp::endpoint_t& in,
                const zmqpp::endpoint_t& out,
                const zmqpp::endpoint_t& bcast,
                const data::Languages& languages)
{
  zmqpp::socket input(ctx, zmqpp::socket_type::pull);
  input.set(zmqpp::socket_option::linger, 1000);
  input.connect(in);

  zmqpp::socket output(ctx, zmqpp::socket_type::push);
  output.set(zmqpp::socket_option::linger, 1000);
  output.connect(out);

  zmqpp::socket kill(ctx, zmqpp::socket_type::sub);
  kill.set(zmqpp::socket_option::subscribe, "KILL");
  kill.connect(bcast);

  bool canContinue = true;
  zmqpp::reactor reactor;
  reactor.add(input, [&input, &output, &languages] () {
      zmqpp::message inMsg;
      input.receive(inMsg);
      std::string msgText;
      inMsg >> msgText;

      data::WikidataElement elem;
      const bool ok = data::parseItem(languages, msgText, elem);
      if (ok) {
        if (processing::hasSameTitles(elem)) {
          output.send("same");
        } else {
          output.send("diff");
        }
      } else {
        output.send("inv");
      }
    });
  reactor.add(kill, [&canContinue] () {
      canContinue = false;
    });

  while (canContinue) {
    reactor.poll(1);
  }
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    std::cout <<
        "Usage: ./wikidata_cpp_main <WIKIDUMP_JSON_FILE> <NUM_WORKERS> "
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

  // Counter
  const zmqpp::endpoint_t counterAddress("inproc://counter");
  zmqpp::socket counter(context, zmqpp::socket_type::pull);
  counter.set(zmqpp::socket_option::linger, 1000);
  counter.bind(counterAddress);

  int numLinesRead = 0;
  int numLinesProcessed = 0;
  int same = 0;
  int different = 0;
  auto counterCallback = [&same, &different, &numLinesRead, &numLinesProcessed, &counter] () {
    zmqpp::message msg;
    counter.receive(msg);
    std::string msgText;
    msg >> msgText;
    if (msgText.compare("same") == 0) {
      ++same;
    } else if (msgText.compare("diff") == 0) {
      ++different;
    }
    // Message is "inv" - just count line
    ++numLinesProcessed;
  };

  // Monitor
  const zmqpp::endpoint_t killAddress("inproc://kill");
  zmqpp::socket killBroadcast(context, zmqpp::socket_type::pub);
  killBroadcast.bind(killAddress);

  const zmqpp::endpoint_t monitorAddress("inproc://monitor");
  zmqpp::socket monitor(context, zmqpp::socket_type::pull);
  monitor.set(zmqpp::socket_option::linger, 1000);
  monitor.bind(monitorAddress);

  bool streamingComplete = false;
  auto streamingCompleteCallback = [&numLinesRead, &streamingComplete, &monitor] () {
    zmqpp::message msg;
    monitor.receive(msg);
    msg >> numLinesRead;
    streamingComplete = true;
  };

  // Reactor
  zmqpp::reactor mainReactor;
  mainReactor.add(counter, counterCallback);
  mainReactor.add(monitor, streamingCompleteCallback);

  // Workers
  const auto workerAddress("inproc://worker");
  std::vector<std::thread> workers;
  for (int i = 0; i < numWorkers; ++i) {
    workers.push_back(std::thread([&context, workerAddress, counterAddress, killAddress, languages] () {
          workerTask(context, workerAddress, counterAddress, killAddress, languages);
        }));
  }

  // Streamer
  auto streamer = std::thread([&context, workerAddress, monitorAddress, fileName] ()
                              {
                                streamerTask(context, workerAddress, monitorAddress, fileName);
                              });

  // Reactor loop
  while (!streamingComplete || (numLinesProcessed < numLinesRead)) {
    mainReactor.poll(10);
  }

  // Send kill signal
  zmqpp::message killMsg;
  killMsg << "KILL" << "Kill!!!";
  killBroadcast.send(killMsg);

  // Cleanup
  streamer.join();
  for (auto& w : workers) {
    w.join();
  }

  const auto t1 = utils::tick();
  io::printResults(same, different, utils::tock(t1, t0));

  return 0;
}
