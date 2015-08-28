//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#include "WindSystem.h"

#include "WindReceiver.h"
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
  _time += dt * _acceleration;
  ComponentHandle<Transform>      xf;
  ComponentHandle<InstanceShape>  instance;
  ComponentHandle<WindReceiver>   wind;
  for (auto __unused e : entities.entities_with_components(xf, instance, wind))
  {
    auto noise = glm::simplex(xf->position() * _scale + vec3(0, 0, _time));
    instance->_openness = mix(instance->_openness, glm::clamp(noise, 0.0f, 1.0f), wind->_influence());
  }
}
