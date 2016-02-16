/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of the wikidata_cpp project:
 *     https://github.com/radupopescu/wikidata_cpp
 */

#include <zmqpp/zmqpp.hpp>

#include <iostream>
#include <string>
#include <thread>

void pipelineTask(const zmqpp::context_t& ctx,
                  const zmqpp::endpoint_t& inPoint,
                  const zmqpp::endpoint_t& outPoint)
{
  zmqpp::socket_t input(ctx, zmqpp::socket_type::pull);
  input.connect(inPoint);

  zmqpp::socket_t output(ctx, zmqpp::socket_type::push);
  output.bind(outPoint);

  while (true) {
    zmqpp::message_t msg;
    input.receive(msg);
    std::string msgText;
    msg >> msgText;
    std::cout << "Task: " << msgText << std::endl;
    output.send(msgText);
    if (msgText.compare("END") == 0) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void endTask(const zmqpp::context_t& ctx,
             const zmqpp::endpoint_t& inPoint)
{
  zmqpp::socket_t input(ctx, zmqpp::socket_type::pull);
  input.connect(inPoint);

  while (true) {
    zmqpp::message_t msg;
    input.receive(msg);
    std::string msgText;
    msg >> msgText;
    std::cout << "Final: " << msgText << std::endl;
    if (msgText.compare("END") == 0) {
      break;
    }
  }
}

int main(int /*argc*/, char** /*argv*/)
{
  zmqpp::context_t context;

  const zmqpp::endpoint_t point0("inproc://point0");
  const zmqpp::endpoint_t pointFinal("inproc://pointFinal");

  zmqpp::socket_t pipelineIn(context, zmqpp::socket_type::push);
  pipelineIn.bind(point0);

  auto t1 = std::thread([&context, point0, pointFinal] () { pipelineTask(context, point0, pointFinal); });
  auto t2 = std::thread([&context, pointFinal] () { endTask(context, pointFinal); });

  for (int i = 0; i < 10; ++i) {
    pipelineIn.send(std::to_string(i));
  }
  pipelineIn.send(std::string("END"));

  t1.join();
  t2.join();

  return 0;
}
