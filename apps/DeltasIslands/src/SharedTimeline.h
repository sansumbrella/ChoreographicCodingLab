//
//  SharedTimeline.h
//
//

#pragma once
#include "Choreograph.h"

namespace sansumbrella
{

/// Returns a shared instance of a timeline.
inline ch::Timeline& sharedTimeline()
{
  static auto timeline = ch::Timeline();
  return timeline;
}

} // namespace sansumbrella
