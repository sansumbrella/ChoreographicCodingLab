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
  uint32_t              _id;
  std::vector<ci::vec2> _points;
};

struct Touch
{
  std::vector<ci::vec2> _points;
};

class IslandDrawingApp : public App {
public:
	void setup() override;
  void touchesBegan( TouchEvent event ) override;
  void touchesMoved( TouchEvent event ) override;
  void touchesEnded( TouchEvent event ) override;

	void update() override;
	void draw() override;

  void createPath(const std::vector<ci::vec2> &points);
  void broadcastPath(const Path &path);
  vec2 normalizePosition(const ci::vec2 &position);

private:
  std::vector<Path>                   _paths;
  std::unordered_map<uint32_t, Touch> _touches;
  const uint32_t                      _max_paths = 7;
  uint32_t                            _current_id = 0;

  shared_ptr<JsonClient> _client;
  shared_ptr<JsonServer> _server;
};

vec2 IslandDrawingApp::normalizePosition(const ci::vec2 &position)
{
  return position / vec2(getWindowSize());
}

void IslandDrawingApp::setup()
{
  _paths.push_back(Path{0, {vec2(10.0f), vec2(0.0f)} });
  _paths.push_back(Path{1, {vec2(10.0f), vec2(0.0f)} });

  auto port = 9191;
  _server = make_shared<JsonServer>( port );
  _server->start();
  _client = make_shared<JsonClient>( io_service() );
  _client->getSignalConnected().connect( [] (bool success) {
    CI_LOG_I("Client connected to server.");
  });

  _client->getSignalDataReceived().connect( [] (const ci::JsonTree &json) {
    CI_LOG_I("Received json, " << json.getChild("type").getValue());
    CI_LOG_I(json);
    if (json.hasChild("points"))
    {
      auto c = json.getChild("points").getNumChildren();
      CI_LOG_I("Json contains " << c << " points.");
    }
  });

  _client->connect(System::getIpAddress(), port);

  getWindow()->getSignalKeyDown().connect([this] (const KeyEvent &event) {
    if (event.getCode() == KeyEvent::KEY_t)
    {
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
    }
  });
}

void IslandDrawingApp::createPath(const std::vector<ci::vec2> &points)
{
  if (_current_id >= _paths.size())
  {
    auto p = Path{_current_id, points};
    _paths.push_back(p);
  }
  else
  {
    _paths.at(_current_id)._points = points;
  }

  broadcastPath(_paths.at(_current_id));
  _current_id = (_current_id + 1) % _max_paths;
}

void IslandDrawingApp::broadcastPath(const Path &path)
{
  JsonTree json;
  json.pushBack(JsonTree("type", "path"));
  json.pushBack(JsonTree("id", path._id));
  auto arr = JsonTree::makeArray("points");
  for (auto &p : path._points)
  {
    auto np = normalizePosition(p);
    auto obj = JsonTree::makeObject();
    obj.addChild(JsonTree("x", np.x));
    obj.addChild(JsonTree("y", np.y));
    arr.pushBack(obj);
  }
  json.pushBack(arr);
  _server->sendMessage(json);
}

void IslandDrawingApp::touchesBegan(cinder::app::TouchEvent event)
{
  for (auto &touch : event.getTouches())
  {
    auto t = Touch();
    t._points.push_back(touch.getPos());
    _touches[touch.getId()] = t;
  }
}

void IslandDrawingApp::touchesMoved(cinder::app::TouchEvent event)
{
  for (auto &touch : event.getTouches())
  {
    auto &t = _touches[touch.getId()];
    auto pos = touch.getPos();
    auto d = distance(t._points.back(), pos);
    if (d > 4.0f)
    {
      t._points.push_back(pos);
    }
  }
}

void IslandDrawingApp::touchesEnded(cinder::app::TouchEvent event)
{
  for (auto &touch : event.getTouches())
  {
    auto &t = _touches[touch.getId()];
    auto pos = touch.getPos();
    auto d = distance(t._points.back(), pos);
    if (d > 4.0f)
    {
      t._points.push_back(pos);
    }

    createPath(t._points);
    _touches.erase(touch.getId());
  }
}

void IslandDrawingApp::update()
{
}

void IslandDrawingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

  for(auto &pair : _touches)
  {
    gl::begin(GL_LINE_STRIP);
    const auto &touch = pair.second;
    for (auto &p : touch._points)
    {
      gl::vertex(p);
    }
    gl::end();
  }

}

CINDER_APP( IslandDrawingApp, RendererGl, [] (App::Settings *settings) {
#if defined(CINDER_COCOA_TOUCH)
#else
  settings->setWindowSize(768, 1024);
#endif
  settings->setMultiTouchEnabled();
} )
