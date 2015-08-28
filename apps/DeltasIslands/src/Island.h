//
//  Island.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once

namespace sansumbrella
{

struct Island
{
  Island() = default;
  Island(uint32_t id): _id(id) {}

  uint32_t  _id = 0;
};

} // namespace sansumbrella
