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
private:
  ci::CameraPersp							_camera;
  ci::CameraUi								_camera_ui;
  std::vector<CameraSettings>	_settings;
  int													_current_setting_index = -1;
  ch::Output<CameraSettings>	_current_settings;
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