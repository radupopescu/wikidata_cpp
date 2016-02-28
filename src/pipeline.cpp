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
                const zmqpp::endpoint_t& logging,
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

  zmqpp::socket logger(ctx, zmqpp::socket_type::pub);
  logger.connect(logging);

  bool canContinue = true;
  zmqpp::reactor reactor;
  reactor.add(input, [&input, &output, &logger, &languages] () {
      zmqpp::message inMsg;
      input.receive(inMsg);
      std::string msgText;
      inMsg >> msgText;

      data::WikidataElement elem;
      const bool ok = data::parseItem(languages, msgText, elem);
      if (ok) {
        zmqpp::message logMsg;
        logMsg << "Logging" << elem.id;
        logger.send(logMsg);
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

void loggerTask(const zmqpp::context& ctx,
                const zmqpp::endpoint_t& logging,
                const zmqpp::endpoint_t& notifications,
                const int loggingInterval)
{
  zmqpp::socket logger(ctx, zmqpp::socket_type::sub);
  logger.set(zmqpp::socket_option::subscribe, "Logging");
  logger.bind(logging);

  zmqpp::socket kill(ctx, zmqpp::socket_type::sub);
  kill.set(zmqpp::socket_option::subscribe, "Notifications");
  kill.connect(notifications);

  int index = 0;
  bool canContinue = true;
  zmqpp::reactor reactor;
  reactor.add(logger, [&logger, &index, loggingInterval] ()
  {
    zmqpp::message msg;
    logger.receive(msg);
    if (index % loggingInterval == 0) {
      std::string topic;
      std::string text;
      msg >> topic >> text;
      std::cout << text << std::endl;
    }
    ++index;
  });
  reactor.add(kill, [&canContinue] ()
  {
    canContinue = false;
  });

  while (canContinue) {
    reactor.poll(1);
  }
}

}

