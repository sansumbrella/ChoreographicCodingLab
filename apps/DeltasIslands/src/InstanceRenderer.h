//
//  Copyright (c) 2015 David Wicks, sansumbrella.com.
//  All rights reserved.
//

#pragma once

#include "entityx.h"
#include "BezierMesh.h"

namespace sansumbrella
{

class InstanceRenderer;
using InstanceRendererRef = std::shared_ptr<InstanceRenderer>;

///
/// Renders instanced bezier geometry.
///
class InstanceRenderer : public entityx::System<InstanceRenderer>
{
public:
  InstanceRenderer();
  ~InstanceRenderer();

  void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) override;
  void draw() const;

  struct InstanceData
  {
    ci::mat4 transform;
    float		 openness;
  };

private:
  size_t													_shape_index = 0;
  std::vector<BezierMesh>         _shapes;
  ci::gl::VboRef									_instance_buffer;
  std::vector<InstanceData>       _instance_data;
};

} // namespace sansumbrella
