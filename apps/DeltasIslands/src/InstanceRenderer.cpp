//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#include "InstanceRenderer.h"
#include "FormBuilders.h"
#include "Transform.h"
#include "InstanceShape.h"

using namespace entityx;
using namespace sansumbrella;
using namespace cinder;

InstanceRenderer::InstanceRenderer()
{
  auto shader = gl::GlslProg::create(gl::GlslProg::Format().vertex(app::loadAsset("glsl/bezier.vs")).fragment(app::loadAsset("glsl/passthrough.fs")));

  _instance_buffer = gl::Vbo::create(GL_ARRAY_BUFFER, 1000 * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
  auto instance_layout = geom::BufferLayout();
  instance_layout.append(geom::Attrib::CUSTOM_8, 16, sizeof(InstanceData), offsetof(InstanceData, transform), 1);
  instance_layout.append(geom::Attrib::CUSTOM_9, 1, sizeof(InstanceData), offsetof(InstanceData, openness), 1);

  _shapes.push_back(createUmbrella(shader, -0.40f, _instance_buffer, instance_layout));
  _shapes.push_back(createWing(shader, 0.5f, _instance_buffer, instance_layout));

  _shape_index = _shapes.size() - 1;
}

InstanceRenderer::~InstanceRenderer()
{}

void InstanceRenderer::update( EntityManager &entities, EventManager &events, TimeDelta dt )
{
  _instance_data.clear();

  ComponentHandle<Transform>      xf;
  ComponentHandle<InstanceShape>  instance;
  for (auto e : entities.entities_with_components(xf, instance))
  {
    _instance_data.emplace_back(InstanceData{ xf->transform(), instance->_openness });
  }

  _instance_buffer->bufferSubData(0, sizeof(InstanceData) * _instance_data.size(), _instance_data.data());
}

void InstanceRenderer::draw() const
{
  gl::ScopedDepth depth(true);
  auto &shape = _shapes.at(_shape_index);

  shape.panels()->getGlslProg()->uniform("uColor", ColorA::gray(1.0f));
  shape.panels()->drawInstanced(_instance_data.size());

  shape.rods()->getGlslProg()->uniform("uColor", ColorA::gray(0.6f));
  shape.rods()->drawInstanced(_instance_data.size());

  gl::drawSolidCircle( vec2(0), 1.0f, 32 );
}
