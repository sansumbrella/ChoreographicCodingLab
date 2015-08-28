//
//  WindReceiver.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once
#include "choreograph/Choreograph.h"

namespace sansumbrella
{

struct WindReceiver
{
  WindReceiver() = default;

  ch::Output<float>     _influence = 1.0f;
};

} // namespace sansumbrella
