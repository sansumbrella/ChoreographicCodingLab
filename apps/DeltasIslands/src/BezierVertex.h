//
//  BezierVertex.h
//
//

#pragma once

namespace sansumbrella
{

struct alignas(16) BezierVertex {
	explicit BezierVertex(const ci::vec3 &pos)
	: p0(pos), p1(pos), c0(pos), c1(pos)
	{}

	explicit BezierVertex(const ci::vec3 &p0, const ci::vec3 &p1, const ci::vec3 &c0, const ci::vec3 &c1, float weight)
	: p0(p0), p1(p1), c0(c0), c1(c1), weight(weight)
	{}

	explicit BezierVertex(const ci::vec3 &p0, const ci::vec3 &p1)
	: p0(p0), p1(p1), c0(p0), c1(p1)
	{}

	BezierVertex& setNormals(const ci::vec3 &normal_0, const ci::vec3 &normal_1) { n0 = normal_0; n1 = normal_1; return *this; }
	BezierVertex& setControlPoint(const ci::vec3 &cp) { c0 = c1 = cp; return *this; }
	BezierVertex& setControlPoint0(const ci::vec3 &cp) { c0 = cp; return *this; }
	BezierVertex& setControlPoint1(const ci::vec3 &cp) { c1 = cp; return *this; }

	BezierVertex& flipNormals() { n0 *= -1.0f; n1 *= -1.0f; return *this; }

	ci::vec3	p0, p1;
	ci::vec3	c0, c1;
	ci::vec3	n0, n1;
	// how far along curve can this point move
	float			weight = 1.0f;
};

const auto BezierBufferLayout = ([] {
	namespace geom = cinder::geom;
	auto layout = geom::BufferLayout();
	layout.append(geom::Attrib::CUSTOM_0, 3, sizeof(BezierVertex), offsetof(BezierVertex, p0));
	layout.append(geom::Attrib::CUSTOM_1, 3, sizeof(BezierVertex), offsetof(BezierVertex, c0));
	layout.append(geom::Attrib::CUSTOM_2, 3, sizeof(BezierVertex), offsetof(BezierVertex, c1));
	layout.append(geom::Attrib::CUSTOM_3, 3, sizeof(BezierVertex), offsetof(BezierVertex, p1));
	layout.append(geom::Attrib::CUSTOM_4, 1, sizeof(BezierVertex), offsetof(BezierVertex, weight));
	layout.append(geom::Attrib::NORMAL, 3, sizeof(BezierVertex), offsetof(BezierVertex, n0));
	layout.append(geom::Attrib::CUSTOM_5, 3, sizeof(BezierVertex), offsetof(BezierVertex, n1));
	return layout;
} ());

const auto BezierAttributeMapping = ([] {
	using namespace cinder;
	return gl::Batch::AttributeMapping{ { geom::Attrib::CUSTOM_0, "A" },
																			{ geom::Attrib::CUSTOM_1, "B" },
																			{ geom::Attrib::CUSTOM_2, "C" },
																			{ geom::Attrib::CUSTOM_3, "D" },
																			{ geom::Attrib::CUSTOM_4, "CurveWeight" },
																			{ geom::Attrib::NORMAL, "NormalBegin" },
																			{ geom::Attrib::CUSTOM_5, "NormalEnd" },
																			{ geom::Attrib::CUSTOM_8, "Transform" },
																			{ geom::Attrib::CUSTOM_9, "Activation" } };
} ());

inline BezierVertex operator * (const ci::mat4 &lhs, const BezierVertex &rhs)
{
	using namespace cinder;
	auto p0 = vec3(lhs * vec4(rhs.p0, 1));
	auto p1 = vec3(lhs * vec4(rhs.p1, 1));
	auto c0 = vec3(lhs * vec4(rhs.c0, 1));
	auto c1 = vec3(lhs * vec4(rhs.c1, 1));
	auto n0 = vec3(lhs * vec4(rhs.n0, 0));
	auto n1 = vec3(lhs * vec4(rhs.n1, 0));

	return BezierVertex(p0, p1, c0, c1, rhs.weight).setNormals(n0, n1);
}

} // namespace sansumbrella
