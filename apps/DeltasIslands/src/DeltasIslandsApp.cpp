#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"

#include "entityx/entityx.h"
#include "InstanceRenderer.h"
#include "WindSystem.h"
#include "InstanceShape.h"
#include "Transform.h"
#include "cinder/Path2d.h"
#include "CameraController.h"

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
private:
  CameraController       _camera;
  entityx::EventManager	 _events;
  entityx::EntityManager _entities;
  entityx::SystemManager _systems;

  vector<Entity>         _island;

  Timer                  _timer;
};

DeltasIslandsApp::DeltasIslandsApp()
: _entities(_events),
  _systems(_entities, _events)
{}

void DeltasIslandsApp::setup()
{
  _systems.add<InstanceRenderer>();
  _systems.add<WindSystem>();
  _systems.configure();

  createTestIsland();
}

void DeltasIslandsApp::createTestIsland()
{
  auto path = Path2d();
  auto pos = vec2(0);
  path.moveTo(pos);
  auto len = 5.0f;
  path.quadTo(pos + vec2(1, 1) * len, pos + vec2(2, 0) * len);

  _island = createIslandFromPath(_entities, path);
  auto i = 0.0f;
  for (auto e : _island)
  {
    auto xf = e.component<Transform>();
    auto t = i / (_island.size() - 1.0f);
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
    case KeyEvent::KEY_r:
      mapIslandToPath(gatherIsland(_entities, 0), randomPath());
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

  auto state = serializePositions(_entities);
  if (getElapsedFrames() % 100) {
    ofstream f((getDocumentsDirectory() / "state.csv").string());
    f << state;
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
