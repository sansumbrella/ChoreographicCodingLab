#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "entityx/entityx.h"
#include "InstanceRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace sansumbrella;
using namespace std;

class DeltasIslandsApp : public App {
public:
  DeltasIslandsApp();
  void setup() override;
  void mouseDown( MouseEvent event ) override;
  void update() override;
  void draw() override;
private:
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

  _systems.update<InstanceRenderer>(dt);
}

void DeltasIslandsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

  _systems.system<InstanceRenderer>()->draw();
}

CINDER_APP( DeltasIslandsApp, RendererGl, [] (App::Settings *settings) {
  settings->setWindowSize(1280, 720);
} )
