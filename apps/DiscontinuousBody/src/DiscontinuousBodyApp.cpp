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
  CameraPersp camera;
  CameraUi    camera_ui;
  gl::VboRef    positions;
  gl::BatchRef  ribbon;
};

void DiscontinuousBodyApp::setup()
{
  camera = CameraPersp(getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f);
  camera_ui.setCamera(&camera);
  camera_ui.connect(getWindow());

  camera.lookAt(vec3(0, 0, -50.0f), vec3(0), vec3(0, 1, 0));
  /*
  positions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec3) * 500);
  auto layout = geom::BufferLayout();
  layout.append(geom::Attrib::POSITION, 3, 0, 0);
  auto mesh = gl::VboMesh::create(500, GL_TRIANGLE_STRIP, {{layout, positions}});

  ribbon = gl::Batch::create(mesh, gl::getStockShader(gl::ShaderDef().color()));
  */
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

  vector<vec3> points;
  for (auto i = 0; i < 100; i += 1)
  {
    auto t = i / 100.0f;
    auto x = mix(0.0f, 4.0f, t);
    auto y = sin(t * M_PI * 4.0f);
    points.push_back(vec3(x, y, 0));
  }

  auto ribbon = sansumbrella::createRibbon(0.2f, ch::EaseOutCubic(), camera.getViewDirection(), points);

  gl::begin(GL_TRIANGLE_STRIP);
  for (auto &p : ribbon)
  {
    gl::vertex(p);
  }
  gl::end();
}

CINDER_APP( DiscontinuousBodyApp, RendererGl )
