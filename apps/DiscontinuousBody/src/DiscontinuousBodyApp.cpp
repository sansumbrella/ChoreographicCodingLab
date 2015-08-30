#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/Shader.h"

#include "choreograph/Choreograph.h"

#include "RibbonFunctions.h"

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
  CameraPersp _camera;
  CameraUi    _camera_ui;
  gl::GlslProgRef _shader;
};

void DiscontinuousBodyApp::setup()
{
  _camera = CameraPersp(getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f);
  _camera_ui.setCamera(&_camera);
  _camera_ui.connect(getWindow());

  _camera.lookAt(vec3(0, 0, -50.0f), vec3(0), vec3(0, 1, 0));
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
  gl::setMatrices(_camera);
  gl::enableDepthRead();

  gl::drawColorCube(vec3(0), vec3(1));

  vector<vec3> points;
  for (auto i = 0; i < 100; i += 1)
  {
    auto t = i / 100.0f;
    auto x = mix(0.0f, 4.0f, t);
    auto y = sin(t * M_PI * 4.0f + getElapsedSeconds() * 3.0f);
    points.push_back(vec3(x, y, 0));
  }

  auto ribbon = sansumbrella::createRibbon(0.2f, ch::EaseOutCubic(), _camera.getViewDirection(), points);

  gl::begin(GL_TRIANGLE_STRIP);
  for (auto &p : ribbon)
  {
    gl::vertex(p);
  }
  gl::end();
}

CINDER_APP( DiscontinuousBodyApp, RendererGl )
