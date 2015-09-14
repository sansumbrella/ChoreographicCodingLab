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

#include "JsonReceiverUDP.h"
#include "cinder/Log.h"

using namespace sansumbrella;
using namespace std;
using namespace asio;
using namespace asio::ip;

JsonReceiverUDP::JsonReceiverUDP(asio::io_service &io_service)
: _io_service(io_service),
  _socket(_io_service)
{
  asio::error_code ec;
  _socket.open(udp::v4(), ec);
  if (ec)
  {
    CI_LOG_E("Error opening socket: " << ec);
  }
}

bool JsonReceiverUDP::connect(const std::string &server, int port)
{
  _socket.cancel();

  udp::resolver resolver(_io_service);
  udp::resolver::query query(udp::v4(), server, to_string(port));
  auto receiver_endpoint = *resolver.resolve(query);

  auto hello = array<char, 1>{ 0 };
  auto ec = asio::error_code();
  _socket.send_to(asio::buffer(hello), receiver_endpoint, 0, ec);

  if (ec)
  {
    // never seems to be a problem, even if there is no receiving process.
    CI_LOG_E("Error sending initial data: " << ec.message());
    return false;
  }

  listen();
  return true;
}

void JsonReceiverUDP::listen()
{
  udp::endpoint sender_endpoint;

  _socket.async_receive_from(asio::buffer(_received_data), sender_endpoint, [this] (const asio::error_code &ec, size_t bytes_read) {
    if (! ec)
    {
      auto str = string(_received_data.begin(), _received_data.begin() + bytes_read);

      try
      {
        auto json = ci::JsonTree(str);
        _json_received.emit(json);
      }
      catch (const exception &exc)
      {
        CI_LOG_E("Exception parsing JSON" << exc.what());
      }

      listen();
    }
    else
    {
      CI_LOG_W("Error in receive: " << ec.message());
      if (ec != asio::error::operation_aborted)
      {
        listen();
      }
    }
  });
}
