//
//  SculptureComponentBase.h
//
//

#pragma once
#include "BezierVertex.h"

namespace sansumbrella
{

/// Model of a single piece of the sculpture.
/// Two batches: one for lines and one for triangles.
class BezierMesh
{
public:
	BezierMesh(const std::vector<BezierVertex> &rods, const std::vector<BezierVertex> &fabric, const ci::gl::GlslProgRef &shader, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout);
	BezierMesh(const ci::gl::BatchRef &rods, const ci::gl::BatchRef &fabric);

	const auto& rods() const { return _rods; }
	const auto& panels() const { return _fabric; }
private:
	ci::gl::BatchRef _rods;
	ci::gl::BatchRef _fabric;
};

} // namespace sansumbrella
