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
#include "CinderImGui.h"

#include <unordered_map>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace sansumbrella;

const int Port = 9191;

template <typename Number>
ImVec2& operator *= (ImVec2 &lhs, Number rhs)
{
  lhs.x *= rhs;
  lhs.y *= rhs;
  return lhs;
}

struct Path
{
  uint32_t              _id;
  float                 _duration = 0.0f;
  std::vector<ci::vec2> _points;
};

struct Touch
{
  Touch() = default;
  explicit Touch(double now)
  : _start_time(now)
  {}
  double                _start_time;
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

  void createPath(const Touch &touch);
  void broadcastPath(const Path &path);
  vec2 normalizePosition(const ci::vec2 &position) const;

  void connectDebugClient();

  ci::JsonTree cameraMessage() const;

  MouseEvent toMouseEvent(const TouchEvent &event);

private:
  std::vector<Path>                   _paths;
  std::unordered_map<uint32_t, Touch> _touches;
  const uint32_t                      _max_paths = 7;
  uint32_t                            _current_id = 0;
  gl::TextureFontRef                  _font;

  float     _camera_height = 1.0f;
  ci::vec2  _camera_pos;

  shared_ptr<JsonClient> _client;
  shared_ptr<JsonServer> _server;
};

MouseEvent IslandDrawingApp::toMouseEvent(const cinder::app::TouchEvent &event)
{
  auto pt = event.getTouches().front().getPos();
  int mods = 0;
  mods |= MouseEvent::LEFT_DOWN;
  return MouseEvent( getWindow(), MouseEvent::LEFT_DOWN, pt.x, pt.y, mods, 0.0f, 0 );
}

vec2 IslandDrawingApp::normalizePosition(const ci::vec2 &position) const
{
  return position / vec2(getWindowSize());
}

ci::JsonTree IslandDrawingApp::cameraMessage() const
{
  auto np = normalizePosition(_camera_pos);
  auto json = JsonTree();
  json.addChild(JsonTree("type", "camera"));
  json.addChild(JsonTree("x", np.x));
  json.addChild(JsonTree("y", np.y));
  json.addChild(JsonTree("z", _camera_height));

  return json;
}

void IslandDrawingApp::setup()
{
  _font = gl::TextureFont::create(Font("Avenir-Medium", 16.0f));

  _server = make_shared<JsonServer>( Port );
  _server->start();

  _camera_pos = getWindowCenter();

#if defined(CINDER_COCOA_TOUCH)
  auto options = ui::Options();
  auto font = getAssetPath("Cousine-Regular.ttf");
  console() << "Font path: " << font << endl;
  options.font(font, toPixels(21.0f));
  ui::initialize(options);

  auto &style = ImGui::GetStyle();
  style.WindowMinSize = ImVec2(400.0f, 400.0f);
  style.ItemInnerSpacing *= 5.0f;
  style.ItemSpacing *= 5.0f;
  style.TouchExtraPadding = ImVec2(30.0f, 10.0f);
  style.FramePadding *= 20.0f;
#else
  connectDebugClient();
  ui::initialize();
#endif
}

void IslandDrawingApp::connectDebugClient()
{
  _client = make_shared<JsonClient>( io_service() );
  _client->getSignalConnected().connect( [] (bool success) {
    CI_LOG_I("Client connected to server.");
  });

  _client->getSignalDataReceived().connect( [] (const ci::JsonTree &json) {
    CI_LOG_I("Received json, " << json.getChild("type").getValue());
    if (json.hasChild("points"))
    {
      auto c = json.getChild("points").getNumChildren();
      CI_LOG_I("Json contains " << c << " points.");
    }
  });

  _client->connect(System::getIpAddress(), Port);

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

void IslandDrawingApp::createPath(const Touch &touch)
{
  auto duration = float(getElapsedSeconds() - touch._start_time);
  if (_current_id >= _paths.size())
  {
    auto p = Path{_current_id, duration, touch._points};
    _paths.push_back(p);
  }
  else
  {
    auto &p = _paths.at(_current_id);
    p._points = touch._points;
    p._duration = duration;
  }

  broadcastPath(_paths.at(_current_id));
  _current_id = (_current_id + 1) % _max_paths;
}

void IslandDrawingApp::broadcastPath(const Path &path)
{
  JsonTree json;
  json.pushBack(JsonTree("type", "path"));
  json.pushBack(JsonTree("id", path._id));
  json.pushBack(JsonTree("duration", path._duration));
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
  auto me = toMouseEvent(event);
  getWindow()->emitMouseDown(&me);

  for (auto &touch : event.getTouches())
  {
    auto t = Touch(getElapsedSeconds());
    t._points.push_back(touch.getPos());
    _touches[touch.getId()] = t;
  }
}

void IslandDrawingApp::touchesMoved(cinder::app::TouchEvent event)
{
  auto me = toMouseEvent(event);
  getWindow()->emitMouseDrag(&me);

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
  auto me = toMouseEvent(event);
  getWindow()->emitMouseUp(&me);

  for (auto &touch : event.getTouches())
  {
    if (! me.isHandled())
    {
      auto &t = _touches[touch.getId()];
      auto pos = touch.getPos();
      auto d = distance(t._points.back(), pos);
      if (d > 4.0f)
      {
        t._points.push_back(pos);
      }

      createPath(t);
    }
    _touches.erase(touch.getId());
  }
}

void IslandDrawingApp::update()
{
  ui::ScopedWindow window("Camera Controls", toPixels(vec2(400.0f, 400.0f)));
  if (ui::SliderFloat("Height", &_camera_height, -50.0f, 100.0f))
  {
    _server->sendMessage(cameraMessage());
  }
  if (ui::SliderFloat2("Position", &_camera_pos.x, 0.0f, getWindowHeight()))
  {
    _server->sendMessage(cameraMessage());
  }
  float angle;
  ui::SliderAngle("Angle", &angle);
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

  for(auto &path : _paths)
  {
    auto t = (path._id / (float)_max_paths);
    gl::ScopedColor color(Color(CM_HSV, t, 1.0f, 1.0f));
    gl::begin(GL_LINE_STRIP);
    for (auto &p : path._points)
    {
      gl::vertex(p);
    }
    gl::end();
  }

  gl::drawStrokedCircle(_camera_pos, 12.0f);

  _font->drawString("IP: " + System::getIpAddress(), vec2(20, 20));
}

CINDER_APP( IslandDrawingApp, RendererGl, [] (App::Settings *settings) {
#if defined(CINDER_COCOA_TOUCH)
#else
  settings->setWindowSize(768, 1024);
#endif
  settings->setMultiTouchEnabled();
} )
