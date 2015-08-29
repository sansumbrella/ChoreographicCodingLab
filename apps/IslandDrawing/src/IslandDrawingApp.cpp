#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "asio/asio.hpp"

#include "JsonClient.h"
#include "JsonServer.h"
#include "cinder/Utilities.h"
#include "cinder/System.h"
#include "cinder/Rand.h"

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

  shared_ptr<JsonClient> _client;
  shared_ptr<JsonServer> _server;
};

void IslandDrawingApp::setup()
{
  _paths.push_back(Path{"Hello", 0, {vec2(10.0f), vec2(0.0f)} });
  _paths.push_back(Path{"Hello", 1, {vec2(10.0f), vec2(0.0f)} });

  auto port = 9191;
  _server = make_shared<JsonServer>( port );
  _server->start();
  _client = make_shared<JsonClient>( io_service() );
  _client->getSignalConnected().connect( [] (bool success) {
    CI_LOG_I("Client connected to server.");
  });

  _client->getSignalDataReceived().connect( [] (const ci::JsonTree &json) {
    CI_LOG_I("Received json, " << json.getChild("type").getValue());
  });

  _client->connect(System::getIpAddress(), port);

  getWindow()->getSignalKeyDown().connect([this] (const KeyEvent &event) {
    JsonTree json;
    json.pushBack(JsonTree("type", "awesome"));
    auto arr = JsonTree::makeArray("points");

    for (auto i = 0; i < 512; i += 1)
    {
      auto obj = JsonTree::makeObject();
      obj.addChild(JsonTree("x", randFloat()));
      obj.addChild(JsonTree("y", randFloat()));
      arr.pushBack( obj );
    }
    json.pushBack(arr);
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

CINDER_APP( IslandDrawingApp, RendererGl, [] (App::Settings *settings) {
#if defined(CINDER_COCOA_TOUCH)
#else
  settings->setWindowSize(768, 1024);
#endif
} )
