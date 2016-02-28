#pragma once

#include "data.h"

#include <zmqpp/socket.hpp>

namespace pipeline {

void streamerTask(const zmqpp::context& ctx,
                  const zmqpp::endpoint_t& out,
                  const zmqpp::endpoint_t& mon,
                  const std::string& fileName);

void workerTask(const zmqpp::context& ctx,
                const zmqpp::endpoint_t& in,
                const zmqpp::endpoint_t& out,
                const zmqpp::endpoint_t& notifications,
                const zmqpp::endpoint_t& logging,
                const data::Languages& languages);

void loggerTask(const zmqpp::context& ctx,
                const zmqpp::endpoint_t& logging,
                const zmqpp::endpoint_t& notifications,
                const int loggingInterval);
}
