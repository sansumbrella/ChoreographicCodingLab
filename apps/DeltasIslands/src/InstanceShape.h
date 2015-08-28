//
//  InstanceShape.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once

namespace sansumbrella
{

struct InstanceShape
{
  InstanceShape() = default;
  InstanceShape(float openness)
  : _openness( openness )
  {}

  float _openness = 0.0f;
};

} // namespace sansumbrella
