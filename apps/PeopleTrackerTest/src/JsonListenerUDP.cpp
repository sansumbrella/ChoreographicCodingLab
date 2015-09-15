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

#include "JSonListenerUDP.h"
#include "cinder/Log.h"

using namespace std;
using namespace asio;
using namespace asio::ip;
using namespace sansumbrella;

JSonListenerUDP::JSonListenerUDP(asio::io_service &io_service, int port)
: _io_service(io_service),
  _socket(_io_service, udp::endpoint(udp::v4(), port))
{

  wait_for_data();
}

void JSonListenerUDP::wait_for_data()
{
  _socket.async_receive_from(asio::buffer(_received_data), _remote_endpoint, [this] (const asio::error_code &ec, size_t bytes_received) {
    handle_data(ec, bytes_received);
  });
}

void JSonListenerUDP::handle_data(const asio::error_code &ec, size_t bytes_received)
{
  if (! ec)
  {
    auto str = string(_received_data.begin(), _received_data.begin() + bytes_received);

    try
    {
      auto json = ci::JsonTree(str);
      _json_received.emit(json);
    }
    catch (const exception &exc)
    {
      CI_LOG_E("Exception parsing JSON" << exc.what());
    }
  }
  else
  {
    CI_LOG_W("Error receiving data: " << ec.message());
  }
  wait_for_data();
}


