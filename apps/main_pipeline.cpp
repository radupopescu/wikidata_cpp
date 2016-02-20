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

void workerTask(const zmqpp::context_t& ctx,
                const zmqpp::endpoint_t& inPoint,
                const zmqpp::endpoint_t& outPoint,
                const data::Languages& languages)
{
  zmqpp::socket_t input(ctx, zmqpp::socket_type::pull);
  input.set(zmqpp::socket_option::linger, 1000);
  input.connect(inPoint);

  zmqpp::socket_t output(ctx, zmqpp::socket_type::push);
  output.set(zmqpp::socket_option::linger, 1000);
  output.connect(outPoint);

  while (true) {
    zmqpp::message_t inMsg;
    input.receive(inMsg);
    std::string msgText;
    inMsg >> msgText;

    if (msgText.compare("end") == 0) {
      break;
    }

    data::WikidataElement elem;
    const bool ok = data::parseItem(languages, msgText, elem);
    if (ok) {
      if (processing::hasSameTitles(elem)) {
        output.send("same");
      } else {
        output.send("diff");
      }
    }
  }
}

void counterTask(const zmqpp::context_t& ctx,
                 const zmqpp::endpoint_t& inPoint)
{
  zmqpp::socket_t input(ctx, zmqpp::socket_type::pull);
  input.set(zmqpp::socket_option::linger, 1000);
  input.bind(inPoint);

  int same = 0;
  int different = 0;

  while (true) {
    zmqpp::message_t msg;
    input.receive(msg);
    std::string msgText;
    msg >> msgText;
    if (msgText.compare("end") == 0) {
      break;
    } else if (msgText.compare("same") == 0) {
      ++same;
    } else if (msgText.compare("diff") == 0) {
      ++different;
    }
  }

  io::printResults(same, different, 0.0);
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

  io::Streamer streamer(fileName);

  zmqpp::context_t context;

  const zmqpp::endpoint_t streamerAddress("inproc://streamer");
  const zmqpp::endpoint_t workerAddress("inproc://worker");
  const zmqpp::endpoint_t counterAddress("inproc://counter");

  zmqpp::socket_t pipelineIn(context, zmqpp::socket_type::push);
  pipelineIn.set(zmqpp::socket_option::linger, 1000);
  pipelineIn.bind(workerAddress);

  zmqpp::socket_t collectorIn(context, zmqpp::socket_type::push);
  collectorIn.connect(counterAddress);

  auto collector = std::thread([&context, counterAddress] () { counterTask(context, counterAddress); });

  std::vector<std::thread> workers;
  for (int i = 0; i < numWorkers; ++i) {
    workers.push_back(std::thread([&context, workerAddress, counterAddress, languages] () {
          workerTask(context, workerAddress, counterAddress, languages);
        }));
  }

  for(std::string line; streamer.getLine(line);) {
    pipelineIn.send(line);
  }

  pipelineIn.send("end");

  for (auto& w : workers) {
    w.join();
  }

  collectorIn.send("end");
  collector.join();

  const auto t1 = utils::tick();
  std::cout << "Time = " << utils::tock(t1, t0) << std::endl;

  return 0;
}
