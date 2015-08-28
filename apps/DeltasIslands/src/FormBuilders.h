//
//  FormBuilders.h
//
//

#pragma once
#include "BezierMesh.h"

///
/// @file Functions for creating SculptureComponents.
///

namespace sansumbrella
{

BezierMesh createUmbrella(const ci::gl::GlslProgRef &shader, float scale, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout);

BezierMesh createWing(const ci::gl::GlslProgRef &shader, float scale, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout);

} // namespace sansumbrella
