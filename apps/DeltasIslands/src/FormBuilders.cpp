//
//  FormBuilders.cpp
//
//

#include "FormBuilders.h"
#include "MathTypes.h"
#include "cinder/Log.h"

using namespace cinder;
using namespace std;

namespace sansumbrella
{

BezierMesh createUmbrella(const ci::gl::GlslProgRef &shader, float scale, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout)
{
	vector<BezierVertex> rods;
	vector<BezierVertex> panels;
	vector<BezierVertex> ring;

	auto blades = 7;

	auto n = normalize(vec3(0, -scale, 0));
	auto root = BezierVertex(vec3(0)).setNormals(n, n);
	auto end_0 = glm::rotate<float>(vec3(0, 1, 0) * scale, Pi * 0.1f, vec3(0, 0, 1));
	auto end_1 = glm::rotate<float>(end_0, Pi * 0.33f, vec3(0, 0, 1));
	auto control = glm::rotate<float>(end_0, Pi * 0.33f / 2.0f, vec3(0, 0, 1));
	auto revolve = glm::rotate<float>(Tau / blades, vec3(0, 1, 0));
	auto normal_0 = normalize(glm::rotate<float>(end_0, Pi / 2.0f, vec3(0, 0, 1)));
	auto normal_1 = normalize(glm::rotate<float>(end_1, Pi / 2.0f, vec3(0, 0, 1)));

	panels.push_back(root);

	for (auto i = 0; i < blades; i += 1)
	{
		rods.push_back(root);
		auto end = BezierVertex(end_0, end_1).setControlPoint(control).setNormals(normal_0, normal_1);
		rods.push_back(end);
		panels.push_back(end);

		ring.push_back(end);

		end_0 = vec3(revolve * vec4(end_0, 1));
		end_1 = vec3(revolve * vec4(end_1, 1));
		control = vec3(revolve * vec4(control, 1));
		normal_0 = vec3(revolve * vec4(normal_0, 0));
		normal_1 = vec3(revolve * vec4(normal_1, 0));

		ring.push_back(BezierVertex(end_0, end_1).setControlPoint(control).setNormals(normal_0, normal_1));
	}

	panels.push_back(BezierVertex(end_0, end_1).setControlPoint(control).setNormals(normal_0, normal_1));
	rods.insert(rods.end(), ring.begin(), ring.end());

	auto rod_vbo = gl::Vbo::create(GL_ARRAY_BUFFER, rods, GL_STATIC_DRAW);
	auto rod_mesh = gl::VboMesh::create(rods.size(), GL_LINES, {{ BezierBufferLayout, rod_vbo }, { instance_layout, instance_data }});

	auto panel_vbo = gl::Vbo::create(GL_ARRAY_BUFFER, panels, GL_STATIC_DRAW);
  auto panel_mesh = gl::VboMesh::create(panels.size(), GL_TRIANGLE_FAN, {{ BezierBufferLayout, panel_vbo }, { instance_layout, instance_data }});

  return BezierMesh(gl::Batch::create(rod_mesh, shader, BezierAttributeMapping), gl::Batch::create(panel_mesh, shader, BezierAttributeMapping));
}

BezierMesh createWing(const ci::gl::GlslProgRef &shader, float scale, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout)
{
	vector<BezierVertex> rods;
	vector<BezierVertex> panels;

	auto center = BezierVertex(vec3(0));
	auto length = vec3(0, 1, 0) * scale;
	auto open = 0.5f;
	auto closed = 0.05f;
	auto midway = (open + closed) / 2.0f;
	auto end_0 = glm::rotate<float>(length, Pi * closed, vec3(0, 0, 1));
	auto end_1 = glm::rotate<float>(length, Pi * open, vec3(0, 0, 1));
	auto control = glm::rotate<float>(length, Pi * midway, vec3(0, 0, 1));
	auto normal_0 = glm::rotate<float>(vec3(1, 0, 0), Pi * closed, vec3(0, 0, 1));
	auto normal_1 = glm::rotate<float>(vec3(1, 0, 0), Pi * open, vec3(0, 0, 1));
	auto end = BezierVertex(end_0, end_1).setControlPoint(control).setNormals(normal_0, normal_1);

	rods.push_back(center);
	rods.push_back(end);

	auto mirror = normalize(glm::angleAxis<float>(Tau * 0.5f, vec3(0, 1, 0)));
	rods.push_back(glm::mat4_cast(mirror) * center);
	rods.push_back(glm::mat4_cast(mirror) * end);

	auto dual = normalize(glm::angleAxis<float>(Tau * 0.5f, vec3(0, 0, 1)) * glm::angleAxis<float>(Tau * 0.25f, vec3(0, 1, 0)));
	auto mirrored_dual = normalize(dual * mirror);
	rods.push_back(glm::mat4_cast(dual) * center);
	rods.push_back(glm::mat4_cast(dual) * end);

	rods.push_back(glm::mat4_cast(mirrored_dual) * center);
	rods.push_back(glm::mat4_cast(mirrored_dual) * end);

	// triangle between point and its dual
	panels.push_back(center);
	panels.push_back(end);
	panels.push_back(glm::mat4_cast(dual) * end);

	panels.push_back(center);
	panels.push_back(end);
	panels.push_back(glm::mat4_cast(mirrored_dual) * end);

	auto mirrored_end = glm::mat4_cast(mirror) * end;
	panels.push_back(center);
	panels.push_back(mirrored_end);
	panels.push_back(glm::mat4_cast(dual) * mirrored_end);

	panels.push_back(center);
	panels.push_back(mirrored_end);
	panels.push_back(glm::mat4_cast(mirrored_dual) * mirrored_end);

	return BezierMesh(rods, panels, shader, instance_data, instance_layout);
}

BezierMesh createReed(const ci::gl::GlslProgRef &shader, const ci::gl::VboRef &instance_data, const ci::geom::BufferLayout &instance_layout)
{
  vector<BezierVertex> rods;
  vector<BezierVertex> panels;

  auto segments = 7;
  auto inner_open = vec3(-0.08f, 0.5f, 0);
  auto outer_open = vec3( -0.24f, 0, 0);

  auto rotation = glm::rotate<float>(Tau * (1.0f / segments), vec3(0, 1, 0));
  auto inner_normal = vec3(glm::mat4_cast(glm::rotation(vec3(1, 0, 0), normalize(vec3(0, 1, 0) - inner_open))) * vec4(0, 1, 0, 0));
  auto outer_normal = vec3(glm::mat4_cast(glm::rotation(vec3(1, 0, 0), normalize(inner_open - outer_open))) * vec4(0, 1, 0, 0));;

  auto center = BezierVertex(vec3(0), vec3(0, 1, 0)).setNormals(vec3(0, 1, 0), vec3(0, 1, 0));

  auto inner1 = BezierVertex(vec3(-0.25, 0, 0), inner_open).setNormals(vec3(0, 1, 0), inner_normal);
  auto inner2 = rotation * inner1;

  auto outer1 = BezierVertex(vec3(-0.5f, 0, 0), outer_open).setNormals(vec3(0, 1, 0), outer_normal);
  auto outer2 = rotation * outer1;

  // triangle between point and its dual
  for (auto i = 0; i < segments; i += 1) {
    panels.push_back(center);
    panels.push_back(inner1);
    panels.push_back(inner2);

    panels.push_back(inner1);
    panels.push_back(inner2);
    panels.push_back(outer1);

    panels.push_back(inner2);
    panels.push_back(outer2);
    panels.push_back(outer1);

    inner1 = inner2;
    inner2 = rotation * inner2;

    outer1 = outer2;
    outer2 = rotation * outer1;
  }

  // squish?

  return BezierMesh(rods, panels, shader, instance_data, instance_layout);
}

} // namespace sansumbrella
