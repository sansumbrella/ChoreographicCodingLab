#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "entityx/entityx.h"
#include "InstanceRenderer.h"
#include "InstanceShape.h"
#include "Transform.h"
#include "cinder/Path2d.h"
#include "CameraController.h"

#include "SharedTimeline.h"

#include "IslandCreation.h"

using namespace ci;
using namespace ci::app;
using namespace sansumbrella;
using namespace entityx;
using namespace std;

class DeltasIslandsApp : public App {
public:
  DeltasIslandsApp();
  void setup() override;
  void createTestIsland();

  void mouseDown( MouseEvent event ) override;
  void update() override;
  void draw() override;
private:
  CameraController       _camera;
  entityx::EventManager	 _events;
  entityx::EntityManager _entities;
  entityx::SystemManager _systems;

  Timer                  _timer;
};

DeltasIslandsApp::DeltasIslandsApp()
: _entities(_events),
  _systems(_entities, _events)
{}

void DeltasIslandsApp::setup()
{
  _systems.add<InstanceRenderer>();
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

  auto island = createIslandFromPath(_entities, path);
  for (auto e : island)
  {
    auto xf = e.component<Transform>();
  }
}

void DeltasIslandsApp::mouseDown( MouseEvent event )
{
}

void DeltasIslandsApp::update()
{
  auto dt = _timer.getSeconds();
  if (dt == 0.0 || dt > 0.1) {
    dt = 1.0 / 60.0;
  }
  _timer.start();

  sharedTimeline().step(dt);
  _systems.update<InstanceRenderer>(dt);
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
