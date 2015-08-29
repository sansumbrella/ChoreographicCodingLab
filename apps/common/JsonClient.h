//
//  JsonClient.h
//
//  Created by David Wicks on 5/4/15.
//
//

#pragma once

#include "asio/asio.hpp"
#include "cinder/Signals.h"
#include "cinder/Json.h"

namespace sansumbrella {

///
/// Client for receiving FrameData from a JsonServer.
///
class JsonClient
{
public:
	JsonClient( asio::io_service &iIoService );
	~JsonClient();

	void connect( const std::string &iServerAddress, int iPort );
	bool isConnected() const { return connected; }

  auto& getSignalDataReceived() { return _data_signal; }
  auto& getSignalConnected() { return _connection_signal; }

	/// Called when the client receives data from the server.
	void receive( const asio::error_code &iError, size_t iBytes );

private:
	asio::ip::tcp::socket       socket;
	bool												connected = false;
	std::string									serverAddress;
	int													port = 0;

  ci::signals::Signal<void (bool)>                      _connection_signal;
  ci::signals::Signal<void (const ci::JsonTree &json)>  _data_signal;

  void listen(size_t offset);

  int               _bytes_remaining = 0;
  int               _message_size = 0;
  std::vector<char> _message;

  void reconnect(float delay);
};

} // namespace sansumbrella
