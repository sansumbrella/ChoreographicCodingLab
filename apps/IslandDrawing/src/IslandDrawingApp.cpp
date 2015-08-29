#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "asio/asio.hpp"

#include "FrameClient.h"
#include "FrameServer.h"
#include "cinder/Utilities.h"
#include "cinder/System.h"

#include <unordered_map>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace sansumbrella;

struct Path
{
  std::string           _name;
  uint32_t              _id;
  std::vector<ci::vec2> _points;
};

struct Touch
{
  
};

class IslandDrawingApp : public App {
public:
	void setup() override;
  void touchesBegan( TouchEvent event ) override;
  void touchesMoved( TouchEvent event ) override;
  void touchesEnded( TouchEvent event ) override;

	void update() override;
	void draw() override;

private:
  std::vector<Path>                   _paths;
  std::unordered_map<uint32_t, Touch> _touches;

  shared_ptr<FrameClient> _client;
  shared_ptr<FrameServer> _server;
};

void IslandDrawingApp::setup()
{
  _paths.push_back(Path{"Hello", 0, {vec2(10.0f), vec2(0.0f)} });
  _paths.push_back(Path{"Hello", 1, {vec2(10.0f), vec2(0.0f)} });

  auto port = 9191;
  _server = make_shared<FrameServer>( port );
  _server->start();
  _client = make_shared<FrameClient>( io_service() );
  _client->getSignalConnected().connect( [] (bool success) {
    CI_LOG_I("Client connected to server.");
  });

  _client->getSignalDataReceived().connect( [] (const ci::JsonTree &json) {
    CI_LOG_I("Received json: " << json.serialize());
  });

  _client->connect(System::getIpAddress(), port);

  getWindow()->getSignalKeyDown().connect([this] (const KeyEvent &event) {
    JsonTree json;
    json.pushBack(JsonTree("type", "awesome"));
//    auto arr = JsonTree::makeArray("points");
//    arr.pushBack( JsonTree("{'x': 0, 'y': 10, 'z': 100, 'a': 5}") );
//    arr.pushBack( JsonTree("{'x': 0, 'y': 10, 'z': 20, 'a': 2}") );
//    json.pushBack(arr);
    _server->sendMessage(json);
  });
}

void IslandDrawingApp::touchesBegan(cinder::app::TouchEvent event)
{

}

void IslandDrawingApp::touchesMoved(cinder::app::TouchEvent event)
{

}

void IslandDrawingApp::touchesEnded(cinder::app::TouchEvent event)
{

  if (_touches.empty())
  {
    // send out path
    CI_LOG_I("Path complete.");
  }
}

void IslandDrawingApp::update()
{
}

void IslandDrawingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( IslandDrawingApp, RendererGl )
