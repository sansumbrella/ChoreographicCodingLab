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
  size_t            _joint_index = 0;
};

class DiscontinuousBodyApp : public App {
public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;

  void updateRibbons();
  void updateSequence();

private:
  CameraPersp     _camera;
  CameraUi        _camera_ui;
  gl::GlslProgRef _shader;

  vector<Ribbon>  _ribbons;

  gl::BatchRef    _sphere_batch;
  gl::VboRef			_instance_vbo;

  int             _current_frame = 0;
  float           _fps = 4.0f;
  float           _frame_duration = 1.0f / _fps;
  float           _current_time = 0.0f;
  Timer           _frame_timer;

  vector<CCL_MocapJoint>  _joint_list;
  vector<size_t>          _ordered_indices;
  ci::gl::VboRef   _position_buffer;
  ci::gl::BatchRef _batch;

  vec3            currentJointPosition(int joint_index);
  int             numFrames();

  size_t          frameIndex(int frame);
  vector<size_t>  generateOrderedIndices(const std::vector<CCL_MocapJoint> &joints);

};

void DiscontinuousBodyApp::setup()
{
  _camera = CameraPersp(getWindowWidth(), getWindowHeight(), 60.0f, 1.0f, 10000.0f);
  _camera_ui.setCamera(&_camera);
  _camera_ui.connect(getWindow());

  _camera.lookAt(vec3(0, 0, -50.0f), vec3(0), vec3(0, 1, 0));
//  _camera.lookAt(vec3( 0, 500, 0 ), vec3( -1888.450,  142.485, -891.197 ));

  _joint_list = ccl::loadMotionCaptureFromJson(getAssetPath("joint_sequence.json"));
  _camera.lookAt(currentJointPosition(0));

  for (auto i = 0; i < _joint_list.size(); i += 1)
  {
    auto r = Ribbon();
    auto pos = currentJointPosition(i);
    r._spine.assign(16, pos);
    r._joint_index = i;
    r._target = pos;
    _ribbons.push_back(r);
  }

  _ordered_indices = generateOrderedIndices(_joint_list);
}

vector<size_t> DiscontinuousBodyApp::generateOrderedIndices(const std::vector<CCL_MocapJoint> &joints)
{
  auto count = joints.at(0).jointPositions.size();
  vector<size_t> indices;
  indices.reserve(count);
  for (auto i = 0; i < count; i += 1) {
    indices.push_back(i);
  }

  auto sort_joint = 27;
  std::stable_sort(indices.begin(), indices.end(), [&] (size_t lhs, size_t rhs) {
    //
    auto lhs_joint = joints.at(sort_joint).jointPositions.at(lhs);
    auto rhs_joint = joints.at(sort_joint).jointPositions.at(rhs);

    return lhs_joint.x < rhs_joint.x;
  });

  std::stable_sort(indices.begin(), indices.end(), [&] (size_t lhs, size_t rhs) {
    //
    auto lhs_joint = joints.at(sort_joint).jointPositions.at(lhs);
    auto rhs_joint = joints.at(sort_joint).jointPositions.at(rhs);

    return lhs_joint.z < rhs_joint.z;
  });

  return indices;
}

void DiscontinuousBodyApp::keyDown( KeyEvent event )
{
  switch (event.getCode())
  {
    case KeyEvent::KEY_f:
      _camera.lookAt(currentJointPosition(0) - vec3(0, 0, 2000), currentJointPosition(0));
    break;
    default:
    break;
  }
}

void DiscontinuousBodyApp::update()
{
  updateSequence();
  updateRibbons();
}

vec3 DiscontinuousBodyApp::currentJointPosition(int joint_index)
{
  return _joint_list.at(joint_index).jointPositions.at(frameIndex(_current_frame));
}

void DiscontinuousBodyApp::updateSequence()
{
  _current_time += _frame_timer.getSeconds();
  _frame_timer.start();

  if (_current_time > _frame_duration) {
    _current_time -= _frame_duration;
    _current_frame = (_current_frame + 1) % numFrames();
  }
}

int DiscontinuousBodyApp::numFrames()
{
  return _joint_list[0].jointPositions.size();
}

void DiscontinuousBodyApp::updateRibbons()
{
  auto easing = 0.5f;
  for (auto &r: _ribbons)
  {
    auto target = currentJointPosition(r._joint_index);
    const auto no_data_value = -123456;
    if (glm::all(glm::greaterThan(target, vec3(no_data_value))))
    {
      r._target = target;
    }

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

  auto shape_fn = [] (float value) {
    if (value < 0.5f) {
      return ch::easeInQuad(lmap(value, 0.0f, 0.5f, 0.0f, 1.0f));
    }
    else {
      return ch::easeInQuad(lmap(value, 0.5f, 1.0f, 1.0f, 0.0f));
    }
  };
  for (auto &r: _ribbons)
  {
    r._triangles = sansumbrella::createRibbon(12.0f, shape_fn, _camera.getViewDirection(), r._spine);
  }

//  _camera.lookAt(currentJointPosition(0));
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
