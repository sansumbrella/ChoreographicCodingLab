//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#include "InstanceRenderer.h"

using namespace entityx;
using namespace sansumbrella;
using namespace cinder;

namespace {

struct InstanceData
{
  ci::mat4 transform;
  float		 activation;
};
  
} // namespace

InstanceRenderer::InstanceRenderer()
{}

InstanceRenderer::~InstanceRenderer()
{}

void InstanceRenderer::update( EntityManager &entities, EventManager &events, TimeDelta dt )
{
  std::vector<InstanceData> data;

  /*
  for (auto e : entities.entities_with_components<Transform, InstancedShape>())
  {
    auto xf = translate(item.position()) * mat4_cast(item.orientation());
    data.push_back(InstanceData{ xf, item.activation() });
  }
  */

  _instances = data.size();
  _instance_data->bufferSubData(0, sizeof(InstanceData) * _instances, data.data());
}

void InstanceRenderer::draw() const
{
  auto &shape = _shapes.at(_shape_index);

  shape.panels()->getGlslProg()->uniform("uColor", ColorA::gray(1.0f));
  shape.panels()->drawInstanced(_instances);

  shape.rods()->getGlslProg()->uniform("uColor", ColorA::gray(0.6f));
  shape.rods()->drawInstanced(_instances);
}
