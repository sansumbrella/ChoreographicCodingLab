#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

#include "entityx/entityx.h"
#include "InstanceRenderer.h"
#include "WindSystem.h"
#include "InstanceShape.h"
#include "Transform.h"
#include "cinder/Path2d.h"
#include "CameraController.h"
#include "JsonClient.h"

#include "SharedTimeline.h"
#include "DataExport.h"

#include "IslandCreation.h"

#include "CinderImGui.h"

using namespace ci;
using namespace ci::app;
using namespace sansumbrella;
using namespace entityx;
using namespace choreograph;
using namespace std;

class DeltasIslandsApp : public App {
public:
  DeltasIslandsApp();
  void setup() override;
  void createTestIsland();

  void keyDown( KeyEvent event ) override;
  void update() override;
  void draw() override;
  void reloadAssets();
  void handleDataReceived(const ci::JsonTree &data);
  void handlePathData(const ci::JsonTree &data);
  void handleCameraData(const ci::JsonTree &data);
private:
  CameraController       _camera;
  entityx::EventManager	 _events;
  entityx::EntityManager _entities;
  entityx::SystemManager _systems;
  JsonClient             _json_client;

  Timer                  _timer;
  std::string            _state;
  bool                   _show_gui = false;
  std::string            _server_ip = "169.254.164.242";
  int                    _server_port = 9191;
};

DeltasIslandsApp::DeltasIslandsApp()
: _entities(_events),
  _systems(_entities, _events),
  _json_client(io_service())
{}

void DeltasIslandsApp::setup()
{
  _systems.add<InstanceRenderer>();
  _systems.add<WindSystem>();
  _systems.configure();

  ui::initialize();

  _json_client.connect(_server_ip, _server_port); // try our initial settings at least
  _json_client.getSignalConnected().connect([] (bool success) {
    CI_LOG_I("Json Client " << (success ? "Successfully Connected" : "Failed to connect.") );
  });
  _json_client.getSignalDataReceived().connect([this] (const ci::JsonTree &data) {
    handleDataReceived(data);
  });

  createTestIsland();
}

void DeltasIslandsApp::reloadAssets()
{
  _systems.system<InstanceRenderer>()->reloadAssets();
}

void DeltasIslandsApp::handleDataReceived(const ci::JsonTree &data)
{
  if (data.hasChild("type"))
  {
    auto type = data.getChild("type").getValue();
    if (type == "path")
    {
      handlePathData(data);
    }
    else if (type == "camera")
    {
      handleCameraData(data);
    }
  }
}

void DeltasIslandsApp::handlePathData(const ci::JsonTree &data)
{
  try
  {
    auto id = data.getChild("id").getValue<uint32_t>();
    auto points = data.getChild("points");
    auto duration = data.getValueForKey<float>("duration");

    Path2d path;
    for (auto &p : points)
    {
      auto pos = vec2(p.getValueForKey<float>("x"), p.getValueForKey<float>("y"));
      pos = mix(vec2(-20.0f, -25.0f), vec2(20.0f, 25.0f), pos);

      if(path.empty())
      {
        path.moveTo(pos);
      }
      else
      {
        path.lineTo(pos);
      }
    }

    auto island = gatherIsland(_entities, id);
    if (island.empty())
    {
      CI_LOG_I("Creating New Island: " << id);
      island = createIslandFromPath(_entities, path, id, 50);
      animateIslandIntoPosition(island, duration);
    }
    else
    {
      CI_LOG_I("Moving Island: " << id);
      mapIslandToPath(island, path, duration);
    }
  }
  catch (const std::exception &exc)
  {
    CI_LOG_W("Exception parsing path data: " << exc.what());
  }
}

void DeltasIslandsApp::handleCameraData(const ci::JsonTree &data)
{
  try
  {
    {
      auto x = data.getValueForKey<float>("x");
      auto y = data.getValueForKey<float>("height");
      auto z = data.getValueForKey<float>("y");

      auto p2 = mix(vec2(-20.0f), vec2(20.0f), vec2(x, z));
      y = mix(-2.0f, 4.0f, y);

      _camera.setTarget(vec3(p2[0], y, p2[1]));
    }

    {
      auto x = data.getValueForKey<float>("view_x");
      auto y = data.getValueForKey<float>("view_y");
      auto z = data.getValueForKey<float>("view_z");
      _camera.setTargetDirection(vec3(x, y, z));
    }

    {
      auto tilt = data.getValueForKey<float>("tilt");
      _camera.setTargetTilt(tilt);
    }
  }
  catch (const std::exception &exc)
  {
    CI_LOG_W("Exception parsing camera data: " << exc.what());
  }
}

void DeltasIslandsApp::createTestIsland()
{
  auto path = Path2d();
  auto pos = vec2(-2.5f, 0.0f);
  path.moveTo(pos);
  auto len = 5.0f;
  path.quadTo(pos + vec2(1, 1) * len, pos + vec2(2, 0) * len);

  auto island = createIslandFromPath(_entities, path);
  animateIslandIntoPosition(island, 2.0f);
}

void DeltasIslandsApp::keyDown( KeyEvent event )
{
  switch (event.getCode())
  {
    case KeyEvent::KEY_p:
      mapIslandToPath(gatherIsland(_entities, 0), randomPath(), 2.0f);
    break;
    case KeyEvent::KEY_r:
      reloadAssets();
    break;
    case KeyEvent::KEY_g:
      _show_gui = ! _show_gui;
    break;
    case KeyEvent::KEY_f:
      setFullScreen(! isFullScreen());
    break;
    case KeyEvent::KEY_w:
    {
      ofstream f((getDocumentsDirectory() / "state.csv").string());
      f << serializationHeader();
      f << _state;
      _state.clear();
      CI_LOG_I("Finished writing state");
    }
    break;
    default:
    break;
  }
}

void DeltasIslandsApp::update()
{
  auto dt = _timer.getSeconds();
  if (dt == 0.0 || dt > 0.1) {
    dt = 1.0 / 60.0;
  }
  _timer.start();

  sharedTimeline().step(dt);
  _camera.update(dt);
  _systems.update<WindSystem>(dt);
  _systems.update<InstanceRenderer>(dt);

  /*
  _state += "frame\n";
  _state += serializePositions(_entities);
  */

  if (_show_gui)
  {
    if (_json_client.isConnected())
    {
      auto server_info = "Connected to server at: " + _server_ip + ":" + to_string(_server_port);
      ui::Text(server_info.c_str(), "");
    }
    else
    {
      ui::InputText("Server IP:", &_server_ip);
      ui::SameLine();
      ui::InputInt("Server Port:", &_server_port);
      if (ui::Button("Connect"))
      {
        _json_client.connect(_server_ip, _server_port);
      }
    }
  }
}

void DeltasIslandsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
  gl::setMatrices(_camera);

  _systems.system<InstanceRenderer>()->draw();
}

CINDER_APP( DeltasIslandsApp, RendererGl, [] (App::Settings *settings) {
  settings->setWindowSize(1280, 720);
} )
