//
//  CameraController.cpp
//
//  Created by David Wicks on 8/28/15.
//
//


#include "CameraController.h"
#include "CinderImGui.h"
#include "SharedTimeline.h"

#include "cinder/Log.h"

using namespace sansumbrella;
using namespace std;
using namespace cinder;

float verticalFov(float focal_length, float film_height = 24.0f)
{
  return glm::degrees(2.0f * std::atan(film_height / (2.0f * focal_length)));
}

CameraController::CameraController()
: _camera(),
_camera_ui(&_camera)
{
  _camera_ui.connect(app::getWindow(), -1);
  app::getWindow()->getSignalKeyDown().connect([this] (const app::KeyEvent &event) { keyDown(event); });

  _camera.setAspectRatio(app::getWindow()->getAspectRatio());
  _camera.setNearClip(0.1f);
  _camera.setFarClip(500.0f);
  _camera.setWorldUp(vec3(0, 1, 0));
  _camera.lookAt(vec3(0, 3, -50), vec3(0));

  _target_position = _camera.getEyePoint();
}

void CameraController::keyDown(const ci::app::KeyEvent &event)
{
  if (event.getCode() >= app::KeyEvent::KEY_0 && event.getCode() <= app::KeyEvent::KEY_9)
  {
    auto value = event.getCode() - app::KeyEvent::KEY_0;

    if (event.isShiftDown()) {
      value += 10;
      if (event.isControlDown()) {
        value += 10;
      }
    }

    animateToView(value);
  }
}

void CameraController::drawGui()
{
  auto fov = _camera.getFov();
  ui::SliderFloat("Field of View", &fov, 5.0f, 120.0f);
  _camera.setFov(fov);
  ui::Text("drag: tumble");
  ui::Text("opt+drag: pan");
  ui::Text("ctrl+drag: zoom");

  vector<string> labels;
  std::transform(_settings.begin(), _settings.end(), std::back_inserter(labels), [] (const CameraSettings &settings) {
    return settings.name;
  });

  if (ui::ListBox("Camera view", &_current_setting_index, labels, 5)) {
    animateToView(_current_setting_index);
  }
}

void CameraController::update(float dt)
{
  auto p = _camera.getEyePoint();
  auto delta = (_target_position - p) * dt * _animation_speed;
  _camera.setEyePoint(p + delta);
}

void CameraController::animateToView(int index)
{
  if (_settings.empty()) {
    return;
  }

  _current_setting_index = index % _settings.size();
  auto &s = _settings.at(_current_setting_index);

  _current_settings = CameraSettings{"current", _camera.getEyePoint(), _camera.getOrientation(), _camera.getFov()};
  auto distance = glm::distance(s.position, _current_settings().position);
  auto duration = mix(1.0f, 2.0f, glm::clamp(distance / 25.0f, 0.0f, 1.0f));

  sharedTimeline().apply(&_current_settings)
		.then<ch::RampTo>(s, duration, ch::EaseInOutQuad())
		.updateFn([this] (ch::Motion<CameraSettings> &m) {
      _camera.setFov(_current_settings().vertical_fov);
      _camera.setEyePoint(_current_settings().position);
      _camera.setOrientation(_current_settings().orientation);
    });
}