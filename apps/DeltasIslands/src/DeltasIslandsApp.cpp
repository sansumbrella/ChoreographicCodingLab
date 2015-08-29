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
  void handlePathData(const ci::JsonTree &data);
private:
  CameraController       _camera;
  entityx::EventManager	 _events;
  entityx::EntityManager _entities;
  entityx::SystemManager _systems;
  JsonClient             _json_client;

  Timer                  _timer;
  std::string            _state;
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

  _json_client.connect("169.254.164.242", 9191);
  _json_client.getSignalConnected().connect([] (bool success) {
    CI_LOG_I("Json Client " << (success ? "Successfully Connected" : "Failed to connect.") );
  });
  _json_client.getSignalDataReceived().connect([this] (const ci::JsonTree &data) {
    CI_LOG_I("Received Path");
    handlePathData(data);
  });

//  createTestIsland();
}

void DeltasIslandsApp::reloadAssets()
{
  _systems.system<InstanceRenderer>()->reloadAssets();
}

void DeltasIslandsApp::handlePathData(const ci::JsonTree &data)
{
  try
  {
    auto type = data.getChild("type").getValue();
    if (type == "path")
    {
      auto id = data.getChild("id").getValue<uint32_t>();
      auto points = data.getChild("points");

      Path2d path;
      for (auto &p : points)
      {
        auto pos = vec2(p.getValueForKey<float>("x"), p.getValueForKey<float>("y"));
        pos = mix(vec2(-20.0f), vec2(20.0f), pos);

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
        createIslandFromPath(_entities, path, id, 50);
      }
      else
      {
        CI_LOG_I("Moving Island: " << id);
        mapIslandToPath(island, path);
      }
    }
  }
  catch (std::exception &exc)
  {
    CI_LOG_W("Exception parsing path data: " << exc.what());
  }
}

void DeltasIslandsApp::createTestIsland()
{
  auto path = Path2d();
  auto pos = vec2(0);
  path.moveTo(pos);
  auto len = 5.0f;
  path.quadTo(pos + vec2(1, 1) * len, pos + vec2(2, 0) * len);

  auto island = createIslandFromPath(_entities, path);
  auto i = 0.0f;
  for (auto e : island)
  {
    auto xf = e.component<Transform>();
    auto t = i / (island.size() - 1.0f);
         t += randFloat(-0.1f, 0.1f);
    auto delay = mix(0.0f, 2.0f, easeOutQuad(glm::clamp(t, 0.0f, 1.0f)));

    sharedTimeline().apply(&xf->position)
      .set( xf->position() - vec3( 0, 10, 0 ) )
      .hold( delay )
      .then<RampTo>( xf->position(), 0.5f, EaseOutCubic() );
  }
}

void DeltasIslandsApp::keyDown( KeyEvent event )
{
  switch (event.getCode())
  {
    case KeyEvent::KEY_p:
      mapIslandToPath(gatherIsland(_entities, 0), randomPath());
    break;
    case KeyEvent::KEY_r:
      reloadAssets();
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
  _systems.update<WindSystem>(dt);
  _systems.update<InstanceRenderer>(dt);

  _state += "frame\n";
  _state += serializePositions(_entities);
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
