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
#include "glm/gtc/noise.hpp"
#include "WindReceiver.h"

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
  auto count = 100;

  for (auto i = 0; i < count; i += 1) {
    auto t = cache.calcNormalizedTime(i / (count - 1.0f));
    auto p = path.getPosition(t);
    auto tangent = normalize(path.getTangent(t));
    auto normal = vec2(-tangent.y, tangent.x);

    auto pos = p;
    island.emplace_back( createShrub(entities, pos) );
    island.emplace_back( createShrub(entities, pos + normal * randFloat(2.0f)) );
    island.emplace_back( createShrub(entities, pos - normal * randFloat(2.0f)) );
  }

  return island;
}

void mapIslandToPath(const std::vector<entityx::Entity> &entities, const ci::Path2d &path)
{
  auto cache = Path2dCalcCache(path);
  auto i = 0.0f;
  const auto length = entities.size() - 1.0f;
  auto offset = -1.0f;

  for (auto e : entities)
  {
    auto t = cache.calcNormalizedTime(i / length);
    i += 1.0f;
    auto delay_t = t + randFloat(-0.2f, 0.2f);
    auto delay = mix(0.0f, 1.0f, easeOutQuad(glm::clamp(delay_t, 0.0f, 1.0f)));

    auto pos = path.getPosition(t);
    auto tangent = normalize(path.getTangent(t));
    auto normal = vec2(-tangent.y, tangent.x);
    auto duration = 1.0f;

    sharedTimeline().append(positionAnim(e))
      .hold(delay)
      .then<RampTo>(planar(pos + offset * normal * randFloat(0.4f, 1.0f)) + vec3(0, glm::simplex(pos * 0.1f) * 0.2f, 0), duration, EaseInOutCubic());

    auto wind = e.component<WindReceiver>();
    if (wind)
    {
      sharedTimeline().append(&wind->_influence)
        .hold(delay)
        .then<RampTo>(0.0f, 0.1f)
        .holdUntil(delay + duration)
        .then<RampTo>(1.0f, 1.5f);
    }

    offset += 1.0f;
    if (offset > 1.0f) {
      offset = -1.0f;
    }
  }
}

Entity createShrub(entityx::EntityManager &entities, const ci::vec2 &pos)
{
  auto e = entities.create();
  e.assign<Transform>( vec3(pos.x, 0.0f, pos.y) );
  e.assign<InstanceShape>( randFloat() );
  e.assign<WindReceiver>();

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

  return pathThroughPoints(points);
}

ci::Path2d pathThroughPoints(const std::vector<ci::vec2> &waypoints)
{
  auto points = pockets::curveThrough(waypoints);
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
