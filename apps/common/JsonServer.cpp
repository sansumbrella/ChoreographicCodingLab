//
//  JsonServer.cpp
//
//  Created by David Wicks on 5/4/15.
//
//

#include "JsonServer.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"

using namespace sansumbrella;

#pragma mark - FrameConnection
namespace sansumbrella {

class FrameConnection
{
public:
	FrameConnection( asio::io_service &iIoService );

	void broadcast( const ci::JsonTree &data );

	bool isDisconnected() const { return disconnected; }

	asio::ip::tcp::socket& getSocket() { return socket; }
private:
	asio::ip::tcp::socket socket;
	bool			disconnected = false;
};

FrameConnection::FrameConnection( asio::io_service &iIoService )
: socket( iIoService )
{}

void FrameConnection::broadcast( const ci::JsonTree &data )
{
  asio::error_code error;
  auto data_str = data.serialize();
  asio::write( socket, asio::buffer( data_str ), error );

  if (error)
  {
    CI_LOG_I("Error writing to client: " << error.message());
    disconnected = true;
  }
}

} // namespace sansumbrella

#pragma mark - JsonServer

JsonServer::JsonServer( int iPort )
: acceptor( ioService, asio::ip::tcp::endpoint( asio::ip::tcp::v4(), iPort ) ),
	running( false )
{
}

JsonServer::~JsonServer()
{
	stop();
}

void JsonServer::start()
{
	running = true;
  ioService.reset();
	thread = std::thread( [this] { run(); } );
}

void JsonServer::stop()
{
	running = false;
	thread.join();
	ioService.stop();
}

void JsonServer::run()
{
	ci::ThreadSetup ts;
	timer.start();

	listenForClients();

	while( running ) {
		ioService.poll();
		update();

    // sleep in milliseconds
		ci::sleep( 1.0f );
	}
}

void JsonServer::sendMessage( const ci::JsonTree &message )
{
	ioService.post( [this, message] {
		for( auto &s : sockets ) {
			s->broadcast( message );
		}
	} );
}

void JsonServer::listenForClients()
{
	CI_LOG_I( "Listening for clients" );

	auto connection = std::make_shared<FrameConnection>( ioService );
	acceptor.async_accept( connection->getSocket(), [this, connection] (const asio::error_code &err) {
		handleClientConnect( connection );
		if( err ) {
			CI_LOG_E( "Frame client connection error: " << err.message() );
		}
	} );
}

void JsonServer::handleClientConnect( const FrameConnectionRef &iConnection )
{
	CI_LOG_I( "Client Connected" );

	sockets.push_back( iConnection );
	listenForClients();
}

void JsonServer::update()
{
	// Remove disconnected sockets.
	auto begin = std::remove_if( sockets.begin(), sockets.end(), [] (const FrameConnectionRef &fc) {
		return fc->isDisconnected();
	} );
	sockets.erase( begin, sockets.end() );
}
