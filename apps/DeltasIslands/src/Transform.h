//
//  Transform.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once
#include "Choreograph.h"

namespace sansumbrella
{

struct Transform
{
  Transform(const ci::vec3 &position = ci::vec3(0), const ci::vec3 &scale = ci::vec3(1), const ci::quat &orientation = ci::quat())
  : position(position),
    scale(scale),
    orientation(orientation)
  {}

  ch::Output<ci::vec3>	position = ci::vec3(0);
  ch::Output<ci::vec3>	scale = ci::vec3(1);
  ch::Output<ci::quat>	orientation;

  ci::mat4 transform() const { return glm::translate(position()) * glm::toMat4(orientation()) * glm::scale(scale()); }
};

} // namespace sansumbrella
