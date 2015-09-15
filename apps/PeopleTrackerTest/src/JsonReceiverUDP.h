/*
 * Copyright (c) 2015 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include "cinder/Signals.h"
#include "cinder/Json.h"
#include "asio/asio.hpp"

namespace sansumbrella
{

using JsonReceiverUDPURef = std::unique_ptr<class JsonReceiverUDP>;

///
/// Receives JSON over UDP multicast.
///
class JsonReceiverUDP
{
public:
  JsonReceiverUDP(asio::io_service &io_service);
  /// Connect to a UDP server.
  /// Returns true on success, false on failure.
  bool connect(const std::string &server, int port);
  bool connect_multicast(const asio::ip::address_v4 &local_address, const asio::ip::address_v4 &sender_address, int port);

  void listen_as_server(int port);

  auto& getSignalJsonReceived() { return _json_received; }
private:
  asio::io_service                                &_io_service;
  asio::ip::udp::socket                           _socket;
  ci::signals::Signal<void (const ci::JsonTree&)> _json_received;
  asio::ip::udp::endpoint                         _sender_endpoint;

  std::array<char, 2048> _received_data;
  asio::ip::udp::endpoint _remote_endpoint;
  void listen();
  void handle_receive(const asio::error_code &ec, size_t bytes_received);
};

} // namespace sansumbrella
