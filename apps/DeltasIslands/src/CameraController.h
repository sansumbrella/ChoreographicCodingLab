//
//  CameraController.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once

#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "Choreograph.h"

namespace sansumbrella
{

struct CameraSettings
{
  std::string name;
  ci::vec3 position;
  ci::quat orientation;
  float		 vertical_fov = 30.0f;
};

///
/// Keeps a list of camera positions.
/// Animates the camera to target positions.
///
class CameraController
{
public:
  CameraController();

  /// Make this object usable where a CameraPersp is requested.
  operator ci::CameraPersp&() { return _camera; }

  void keyDown(const ci::app::KeyEvent &event);

  void drawGui();
  void animateToView(int index);
  void animateToPosition(const ci::vec3 &position);

  void setTarget(const ci::vec3 &position) { _target_position = position; }
  void setTargetDirection(const ci::vec3 &direction) { _view_direction = direction; }
  void setTargetTilt(float tilt) { _target_tilt = tilt; }

  // in range of 0.1 - ~30
  void setSuddenness(float suddenness) { _animation_speed = suddenness; }

  void update(float dt);
private:
  ci::CameraPersp							_camera;
  ci::CameraUi								_camera_ui;
  std::vector<CameraSettings>	_settings;
  int													_current_setting_index = -1;
  ch::Output<CameraSettings>	_current_settings;

  ci::vec3                    _target_position;
  ci::vec3                    _view_direction;
  float                       _target_tilt = 0.0f;
  float                       _tilt = 0.0f;
  float                       _animation_speed = 0.9f;
};

} // namespace sansumbrella

namespace choreograph
{
template <>
inline sansumbrella::CameraSettings lerpT(const sansumbrella::CameraSettings &start, const sansumbrella::CameraSettings &end, float time)
{
  return {
    .name = end.name,
    .position = lerpT(start.position, end.position, time),
    .orientation = lerpT(start.orientation, end.orientation, time),
    .vertical_fov = lerpT(start.vertical_fov, end.vertical_fov, time)
  };
}

} // namespace choreograph
