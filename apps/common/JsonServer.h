//
//  JsonServer.h
//
//  Created by David Wicks on 5/4/15.
//
//

#pragma once

#include "asio/asio.hpp"
#include "cinder/Json.h"

namespace sansumbrella {

/// A socket connection to a JsonClient.
/// Used internally by JsonServer.
class FrameConnection;
using FrameConnectionRef = std::shared_ptr<FrameConnection>;

///
/// A server for sending FrameData to JsonClients over TCP.
/// Waits until all clients are connected before sending reset.
/// Waits until all clients have rendered a frame before sending update.
/// Runs all commands synchronously, so place in a separate thread (potentially with separate io_service).
///
class JsonServer
{
public:
	JsonServer( int iPort );
	~JsonServer();

	/// Start running a frame update loop.
	void start();
	/// Stop the frame update loop.
	void stop();

	/// Send a message to all connected clients.
  void sendMessage( const ci::JsonTree &message );

private:
	asio::io_service								ioService;
  asio::ip::tcp::acceptor					acceptor;
  std::vector<FrameConnectionRef> sockets;
	std::thread											thread;
	ci::Timer												timer;
	std::atomic_bool								running;

	/// Run the threaded update loop
	void run();

	void listenForClients();
	void handleClientConnect( const FrameConnectionRef &iConnection );
	void update();
};

} // namespace sansumbrella
