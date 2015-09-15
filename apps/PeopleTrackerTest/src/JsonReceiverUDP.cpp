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

void JsonReceiverUDP::listen_as_server(int port)
{
  _socket = udp::socket(_io_service, udp::endpoint(udp::v4(), port));
  _socket.async_receive_from(asio::buffer(_received_data), _remote_endpoint, [this] (const asio::error_code &ec, size_t bytes_read) {
    auto str = string(_received_data.begin(), _received_data.begin() + bytes_read);
    CI_LOG_I("Received data: " << bytes_read);
  });
}

bool JsonReceiverUDP::connect(const std::string &server, int port)
{
  _socket.cancel();

  udp::resolver resolver(_io_service);
  udp::resolver::query query(udp::v4(), server, to_string(port));
  auto receiver_endpoint = *resolver.resolve(query);

  auto ec = asio::error_code();
  _socket.connect(receiver_endpoint, ec);

  if (ec)
  {
    // never seems to be a problem, even if there is no receiving process.
    CI_LOG_E("Error sending initial data: " << ec.message());
    return false;
  }

  listen();
  return true;
}

bool JsonReceiverUDP::connect_multicast(const asio::ip::address_v4 &local_address, const asio::ip::address_v4 &sender_address, int port)
{
  if (_socket.is_open())
  {
    _socket.cancel();
    _socket.close();
  }

  // Create the socket so that multiple may be bound to the same address.
  udp::endpoint listen_endpoint(local_address, port);
  _socket.open(listen_endpoint.protocol());
  _socket.set_option(asio::socket_base::reuse_address( true ));
  _socket.set_option(asio::socket_base::keep_alive( true ));
  _socket.bind( asio::ip::udp::endpoint( sender_address, port ) );

  // Join the multicast group.
  _socket.set_option( asio::ip::multicast::join_group( sender_address ) );
  // Sender endpoint is default initialized (as in the sample); I believe this lets the multicast work by not overriding it.
  _socket.async_receive_from( asio::buffer(_received_data), _sender_endpoint,
                            [this] (const asio::error_code &err, size_t bytes) {
                              handle_receive( err, bytes );
                            } );

  return true;
}

void JsonReceiverUDP::handle_receive(const asio::error_code &ec, size_t bytes_received)
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
    CI_LOG_W("Error receiving udp: " << ec.message());
  }

  if (ec != asio::error::operation_aborted)
  {
    _socket.async_receive_from( asio::buffer(_received_data), _sender_endpoint,
                               [this] (const asio::error_code &err, size_t bytes) {
                                 handle_receive( err, bytes );
                               } );
  }
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
