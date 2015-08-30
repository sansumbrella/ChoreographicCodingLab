#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DiscontinuousBodyApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
private:
  CameraPersp camera;
  CameraUi    camera_ui;

};

void DiscontinuousBodyApp::setup()
{
  camera = CameraPersp(getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f);
  camera_ui.setCamera(&camera);
  camera_ui.connect(getWindow());

  camera.lookAt(vec3(0, 0, -50.0f), vec3(0), vec3(0, 1, 0));
}

void DiscontinuousBodyApp::mouseDown( MouseEvent event )
{
}

void DiscontinuousBodyApp::update()
{
}

void DiscontinuousBodyApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
  gl::setMatrices(camera);
  gl::enableDepthRead();

  gl::drawColorCube(vec3(0), vec3(1));
}

CINDER_APP( DiscontinuousBodyApp, RendererGl )
