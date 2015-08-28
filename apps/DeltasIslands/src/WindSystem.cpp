//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#include "WindSystem.h"

#include "Transform.h"
#include "InstanceShape.h"
#include "glm/gtc/noise.hpp"

using namespace entityx;
using namespace sansumbrella;
using namespace cinder;

WindSystem::WindSystem()
{}

WindSystem::~WindSystem()
{}

void WindSystem::update( EntityManager &entities, EventManager &events, TimeDelta dt )
{
  _time += dt;
  ComponentHandle<Transform>      xf;
  ComponentHandle<InstanceShape>  instance;
  for (auto e : entities.entities_with_components(xf, instance))
  {
    auto noise = glm::simplex(xf->position() + vec3(0, 0, _time));
    instance->_openness = glm::clamp(noise, 0.0f, 1.0f);
  }
}
