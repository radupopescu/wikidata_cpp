#include "pipeline.h"

#include <zmqpp/message.hpp>
#include <zmqpp/reactor.hpp>

#include "io.h"
#include "processing.h"

namespace pipeline {

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
                const zmqpp::endpoint_t& notifications,
                const data::Languages& languages)
{
  zmqpp::socket input(ctx, zmqpp::socket_type::pull);
  input.set(zmqpp::socket_option::linger, 1000);
  input.connect(in);

  zmqpp::socket output(ctx, zmqpp::socket_type::push);
  output.set(zmqpp::socket_option::linger, 1000);
  output.connect(out);

  zmqpp::socket kill(ctx, zmqpp::socket_type::sub);
  kill.set(zmqpp::socket_option::subscribe, "Notifications");
  kill.connect(notifications);

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

}

