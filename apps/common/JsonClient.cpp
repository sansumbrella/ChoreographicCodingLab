//
//  JsonClient.cpp
//
//  Created by David Wicks on 5/4/15.
//
//

#include "JsonClient.h"
#include "cinder/app/App.h"
#include "cinder/Timeline.h"
#include "cinder/Log.h"

using namespace sansumbrella;

JsonClient::JsonClient( asio::io_service &iIoService )
: socket( iIoService )
{
}

JsonClient::~JsonClient()
{
	socket.close();
}

void JsonClient::connect( const std::string &iServerAddress, int iPort )
{
	serverAddress = iServerAddress;
	port = iPort;
	CI_LOG_I( "Attempting to connect to " << iServerAddress << ":" << iPort );

	asio::ip::tcp::resolver resolver( socket.get_io_service() );
	asio::ip::tcp::resolver::query query( iServerAddress, std::to_string(iPort) );
	auto iter = resolver.resolve( query );

	asio::async_connect( socket, iter, [this] (const asio::error_code &error, const asio::ip::tcp::resolver::iterator &iter) {
		if( error ) {
			if( error == asio::error::operation_aborted ) {
				CI_LOG_W( "Aborting previous connection attempt." );
			}
			else {
				CI_LOG_W( "Error connecting to server: " << error.message() << ". Retrying." );
				connected = false;
        reconnect(2.0f);
			}
		}
		else {
			CI_LOG_I( "Connected to server: " << iter->host_name() );
			connected = true;
      _connection_signal.emit(true);
			listen(0);
		}
	});
}

void JsonClient::reconnect(float delay)
{
  ci::app::timeline().add( [this] { connect( serverAddress, port ); }, 3.0 );
}

void JsonClient::receive( const asio::error_code &iError, size_t iBytes )
{
  _bytes_remaining -= iBytes;
	if( iError ) {
		if( iError == asio::error::eof ) {
			// We were disconnected, try to reconnect
			CI_LOG_W( "Lost connection to server." );
		}
		else {
			CI_LOG_E( "Error receiving as client: " << iError.message() );
		}

		socket.close();
		connected = false;
    _connection_signal.emit(false);

    reconnect(5.0f);
	}
	else if( _bytes_remaining == 0 )
	{
    // message.end() is incorrect, since we are writing directly into its data.
    auto str = std::string(_message.begin(), _message.begin() + _message_size);
    try
    {
      auto json = ci::JsonTree(str);
      _data_signal.emit(json);
    }
    catch (std::exception &exc)
    {
      CI_LOG_E("Received invalid json: " << exc.what());
      CI_LOG_I("Bad input string: " << str);
    }

		listen(0);
	}
	else
	{
		CI_LOG_E( "Frame Client received data of incorrect size: " << iBytes );
		listen(_message_size - _bytes_remaining);
	}

}

void JsonClient::listen(size_t offset)
{
  if (offset == 0)
  {
    socket.async_receive( asio::buffer(&_message_size, sizeof(_message_size)), [this] (const asio::error_code &error, size_t bytes) {
      _message.reserve(_message_size);
      _bytes_remaining = _message_size;
      socket.async_receive( asio::buffer( _message.data(), _message_size ), [this] (const asio::error_code &error, size_t bytes) {
        receive(error, bytes);
      });
    });
  }
  else
  {
    CI_LOG_I("Reading some more data into buffer");
    socket.async_receive( asio::buffer( _message.data() + offset, _bytes_remaining ), [this] (const asio::error_code &error, size_t bytes) {
      receive(error, bytes);
    });
  }

}
