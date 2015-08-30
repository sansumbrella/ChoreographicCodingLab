#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/Shader.h"

#include "choreograph/Choreograph.h"

#include "RibbonFunctions.h"
#include "CCL_MocapData.h"

#include "cinder/Rand.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Ribbon
{
  vec3              _target;
  std::vector<vec3> _spine;
  std::vector<vec3> _triangles;
};

class DiscontinuousBodyApp : public App {
public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
private:
  CameraPersp     _camera;
  CameraUi        _camera_ui;
  gl::GlslProgRef _shader;

  vector<Ribbon>  _ribbons;

  gl::BatchRef    _sphere_batch;
  gl::VboRef			_instance_vbo;

  int             _current_frame = 0;

  vector<CCL_MocapJoint>  _joint_list;
  vector<size_t>          _ordered_indices;

  size_t          frameIndex(int frame);
  vector<size_t>  generateOrderedIndices(const std::vector<CCL_MocapJoint> &joints);


};

void DiscontinuousBodyApp::setup()
{
  _camera = CameraPersp(getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f);
  _camera_ui.setCamera(&_camera);
  _camera_ui.connect(getWindow());

  _camera.lookAt(vec3(0, 0, -50.0f), vec3(0), vec3(0, 1, 0));

  _joint_list = ccl::loadMotionCaptureFromJson(getAssetPath("CCL_JOINT.json"));

  auto r = Ribbon();
  for (auto i = 0; i < 12; i += 1)
  {
    auto t = i / 12.0f;
    auto x = mix(0.0f, 4.0f, t);
    auto y = sin(t * M_PI * 4.0f + getElapsedSeconds() * 3.0f);
    r._spine.push_back(vec3(x, y, 0));
  }
  _ribbons.push_back(r);
}

void DiscontinuousBodyApp::keyDown( KeyEvent event )
{
  auto pos = randVec3() * 3.0f;
  for (auto &r : _ribbons)
  {
    r._target = vec3(pos);
  }
}

void DiscontinuousBodyApp::update()
{
  auto easing = 0.5f;
  for (auto &r: _ribbons)
  {
    auto &points = r._spine;
    for (auto i = points.size() - 1; i > 0; i -= 1)
    {
      auto &p1 = points.at(i);
      auto &p2 = points.at(i - 1);
      p1 += (p2 - p1) * easing;
    }
    auto &point = points.at(0);
    point += (r._target - point) * easing;
  }

  for (auto &r: _ribbons)
  {
    r._triangles = sansumbrella::createRibbon(0.2f, ch::EaseOutCubic(), _camera.getViewDirection(), r._spine);
  }
}

void DiscontinuousBodyApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
  gl::setMatrices(_camera);
  gl::enableDepthRead();

  gl::drawColorCube(vec3(0), vec3(1));

  for (auto &ribbon: _ribbons)
  {
    gl::begin(GL_TRIANGLE_STRIP);
    for (auto &p : ribbon._triangles)
    {
      gl::vertex(p);
    }
    gl::end();
  }
}

size_t DiscontinuousBodyApp::frameIndex(int frame)
{
  if (_ordered_indices.empty())
  {
    return frame;
  }
  else if (frame >= _ordered_indices.size())
  {
    CI_LOG_W("Out of range index!");
    return frame;
  }
  else
  {
    return _ordered_indices.at(frame);
  }
}

CINDER_APP( DiscontinuousBodyApp, RendererGl )
