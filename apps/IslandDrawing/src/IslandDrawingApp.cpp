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

  vec2 normalizePosition(const ci::vec2 &position) const;
  vec2 denormalizePosition(const ci::vec2 &position) const;

  void createPath(const Touch &touch);
  void broadcastPath(const Path &path);

  void connectDebugClient();

  ci::JsonTree cameraMessage() const;

  MouseEvent toMouseEvent(const TouchEvent &event) const;

private:
  std::vector<Path>                   _paths;
  std::unordered_map<uint32_t, Touch> _touches;
  const uint32_t                      _max_paths = 7;
  uint32_t                            _current_id = 0;

  uint32_t                            _dragging_touch = 0;

  float     _camera_height = 0.5f;
  ci::vec2  _camera_pos = vec2(0.5f);

  ci::vec2  _view_direction = vec2(0, -1);
  float     _camera_angle = 0.0f;
  float     _camera_tilt = 0.0f;

  shared_ptr<JsonClient> _client;
  shared_ptr<JsonServer> _server;
};

vec2 IslandDrawingApp::normalizePosition(const ci::vec2 &position) const
{
  return position / float(getWindowWidth());
}

vec2 IslandDrawingApp::denormalizePosition(const ci::vec2 &position) const
{
  return position * float(getWindowWidth());
}

MouseEvent IslandDrawingApp::toMouseEvent(const cinder::app::TouchEvent &event) const
{
  auto pt = event.getTouches().front().getPos();
  int mods = 0;
  mods |= MouseEvent::LEFT_DOWN;
  return MouseEvent( getWindow(), MouseEvent::LEFT_DOWN, pt.x, pt.y, mods, 0.0f, 0 );
}

ci::JsonTree IslandDrawingApp::cameraMessage() const
{
  auto json = JsonTree();
  json.addChild(JsonTree("type", "camera"));
  json.addChild(JsonTree("x", _camera_pos.x));
  json.addChild(JsonTree("y", _camera_pos.y));
  json.addChild(JsonTree("height", _camera_height));
  json.addChild(JsonTree("view_x", _view_direction.x));
  json.addChild(JsonTree("view_y", 0));
  json.addChild(JsonTree("view_z", _view_direction.y));

  return json;
}

void IslandDrawingApp::setup()
{
  _server = make_shared<JsonServer>( Port );
  _server->start();

#if defined(CINDER_COCOA_TOUCH)
  auto options = ui::Options();
  auto font = getAssetPath("Cousine-Regular.ttf");
  console() << "Font path: " << font << endl;
  options.font(font, toPixels(21.0f));
  ui::initialize(options);

  auto &style = ImGui::GetStyle();
  style.WindowMinSize = ImVec2(300.0f, 300.0f);
  style.ItemInnerSpacing *= 3.0f;
  style.ItemSpacing *= 5.0f;
  style.TouchExtraPadding = ImVec2(30.0f, 10.0f);
  style.FramePadding *= 12.0f; // this is the magic number for sizing
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

    if (distance(touch.getPos(), denormalizePosition(_camera_pos)) < 24.0f)
    {
      _dragging_touch = touch.getId();
      _camera_pos = normalizePosition(touch.getPos());
      _server->sendMessage(cameraMessage());
    }
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

    if (touch.getId() == _dragging_touch)
    {
      _camera_pos = normalizePosition(touch.getPos());
      _server->sendMessage(cameraMessage());
    }
  }
}

void IslandDrawingApp::touchesEnded(cinder::app::TouchEvent event)
{
  auto me = toMouseEvent(event);
  getWindow()->emitMouseUp(&me);

  for (auto &touch : event.getTouches())
  {
    if (touch.getId() == _dragging_touch)
    {
      _camera_pos = normalizePosition(touch.getPos());
      _server->sendMessage(cameraMessage());
      me.setHandled();
      _dragging_touch = 0;
    }

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
  ui::ScopedWindow window("Camera Controls", toPixels(vec2(400.0f, 250.0f)), 0.5f);
  ui::Text(("IP: " + System::getIpAddress()).c_str(), "");
  if (ui::SliderFloat("Height", &_camera_height, 0.0f, 1.0f))
  {
    _server->sendMessage(cameraMessage());
  }
  if (ui::SliderFloat2("Position", &_camera_pos.x, 0.0f, 1.0f))
  {
    _server->sendMessage(cameraMessage());
  }
  if (ui::SliderFloat("Angle", &_camera_angle, -M_PI, M_PI))
  {
    _view_direction = glm::rotate(vec2(0, -1), _camera_angle);
    _server->sendMessage(cameraMessage());
  }
  /*
  if (ui::SliderAngle("Tilt", &_camera_tilt))
  {
    _server->sendMessage(cameraMessage());
  }
  */
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

  auto pos = denormalizePosition(_camera_pos);
  gl::drawStrokedCircle(pos, 12.0f);
  gl::drawLine(pos, pos + _view_direction * 24.0f);
}

CINDER_APP( IslandDrawingApp, RendererGl, [] (App::Settings *settings) {
#if defined(CINDER_COCOA_TOUCH)
#else
  settings->setWindowSize(768, 1024);
#endif
  settings->setMultiTouchEnabled();
} )
