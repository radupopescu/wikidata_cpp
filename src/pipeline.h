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
                const data::Languages& languages);

}
