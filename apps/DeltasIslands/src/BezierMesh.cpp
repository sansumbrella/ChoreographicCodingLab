//
//  SculptureComponentBase.cpp
//
//

#include "BezierMesh.h"

using namespace sansumbrella;
using namespace cinder;

BezierMesh::BezierMesh(const std::vector<BezierVertex> &rods, const std::vector<BezierVertex> &fabric, const ci::gl::GlslProgRef &shader, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout)
{
	{
		auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER, rods, GL_STATIC_DRAW);
    auto mesh = gl::VboMesh::create(rods.size(), GL_LINES, {{ BezierBufferLayout, vbo }, { instance_layout, instance_data }});
    _rods = gl::Batch::create(mesh, shader, BezierAttributeMapping);
	}

	{
		auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER, fabric, GL_STATIC_DRAW);
    auto mesh = gl::VboMesh::create(fabric.size(), GL_TRIANGLES, {{ BezierBufferLayout, vbo }, { instance_layout, instance_data }});
		_fabric = gl::Batch::create(mesh, shader, BezierAttributeMapping);
	}
}

BezierMesh::BezierMesh(const ci::gl::BatchRef &rods, const ci::gl::BatchRef &fabric)
: _rods(rods),
	_fabric(fabric)
{}
