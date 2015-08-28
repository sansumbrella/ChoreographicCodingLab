//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#pragma once

#include "entityx.h"

namespace sansumbrella
{

class WindSystem;
using WindSystemRef = std::shared_ptr<WindSystem>;

class WindSystem : public entityx::System<WindSystem>
{
public:
  WindSystem();
  ~WindSystem();

  void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) override;
private:
  double _time = 0;
};

} // namespace sansumbrella
