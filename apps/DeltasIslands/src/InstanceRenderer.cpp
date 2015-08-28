//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#include "InstanceRenderer.h"
#include "FormBuilders.h"

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
{
  auto shader = gl::GlslProg::create(gl::GlslProg::Format().vertex(app::loadAsset("glsl/bezier.vs")).fragment(app::loadAsset("glsl/passthrough.fs")));

  _instance_data = gl::Vbo::create(GL_ARRAY_BUFFER, 500 * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
  auto instance_layout = geom::BufferLayout();
  instance_layout.append(geom::Attrib::CUSTOM_8, 16, sizeof(InstanceData), offsetof(InstanceData, transform), 1);
  instance_layout.append(geom::Attrib::CUSTOM_9, 1, sizeof(InstanceData), offsetof(InstanceData, activation), 1);

  _shapes.push_back(createUmbrella(shader, -0.40f, _instance_data, instance_layout));
  _shapes.push_back(createWing(shader, 0.5f, _instance_data, instance_layout));

  _shape_index = _shapes.size() - 1;
}

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
