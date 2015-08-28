//
//  IslandCreation.cpp
//
//  Created by David Wicks on 8/28/15.
//
//

#include "IslandCreation.h"
#include "Transform.h"
#include "InstanceShape.h"
#include "SharedTimeline.h"
#include "cinder/Rand.h"
#include "CurveUtilities.h"

using namespace cinder;
using namespace entityx;
using namespace std;
using namespace choreograph;

namespace sansumbrella
{
  namespace
  {

    ch::Output<vec3>* positionAnim(entityx::Entity &e)
    {
      return &e.component<Transform>()->position;
    }

    vec3 planar(const ci::vec2 &pos)
    {
      return vec3(pos.x, 0, pos.y);
    }

  } // namespace

vector<Entity> createIslandFromPath(entityx::EntityManager &entities, const ci::Path2d &path)
{
  auto cache = Path2dCalcCache(path);
  auto island = vector<Entity>();

  for (auto i = 0; i < 10; i += 1) {
    auto t = cache.calcNormalizedTime(i / 10.0f);
    auto p = path.getPosition(t);
    auto tangent = normalize(path.getTangent(t));
    auto normal = vec2(-tangent.y, tangent.x);

    auto pos = p;
    island.emplace_back( createShrub(entities, pos) );
    island.emplace_back( createShrub(entities, pos + normal * 2.0f) );
    island.emplace_back( createShrub(entities, pos - normal * 2.0f) );
  }

  return island;
}

void mapIslandToPath(const std::vector<entityx::Entity> &entities, const ci::Path2d &path)
{
  auto cache = Path2dCalcCache(path);
  auto i = 0.0f;
  const auto length = entities.size() - 1.0f;
  auto offset = -1.0f;
  auto delay = 0.0f;

  for (auto e : entities)
  {
    auto t = cache.calcNormalizedTime(i / length);
    i += 1.0f;

    auto pos = path.getPosition(t);
    auto tangent = normalize(path.getTangent(t));
    auto normal = vec2(-tangent.y, tangent.x);

    sharedTimeline().apply(positionAnim(e))
      .hold(delay)
      .then<RampTo>(planar(pos + offset * normal), 0.5f, EaseInOutCubic());

    offset += 1.0f;
    if (offset > 1.0f) {
      offset = -1.0f;
    }
    delay += 0.05f;
  }
}

Entity createShrub(entityx::EntityManager &entities, const ci::vec2 &pos)
{
  auto e = entities.create();
  e.assign<Transform>( vec3(pos.x, 0.0f, pos.y) );
  e.assign<InstanceShape>( randFloat() );

  return e;
}

ci::Path2d randomPath()
{
  vector<vec2> points;
  auto pos = vec2(randFloat(-20.0f, 20.0f), randFloat(-20.0f, 20.0f));
  for (auto i = 0; i < 5; i += 1) {
    points.push_back(pos);
    pos += vec2(randFloat(-1.0f, 1.0f) * 10.0f, randFloat(-1.0f, 1.0f) * 10.0f);
  }

  points = pockets::curveThrough(points);
  auto path = Path2d();
  const auto stride = 4;

  for (auto i = 0; i < points.size(); i += stride)
  {
    if (i == 0) {
      path.moveTo(points.at(i));
    }
    path.curveTo(points.at(i + 1), points.at(i + 2), points.at(i + 3));
  }

  return path;
}

} // namespace sansumbrella
