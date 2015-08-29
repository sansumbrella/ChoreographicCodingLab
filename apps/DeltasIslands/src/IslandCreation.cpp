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
#include "Island.h"
#include "cinder/Log.h"

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

vector<Entity> createIslandFromPath(entityx::EntityManager &entities, const ci::Path2d &path, uint32_t island_id, int count)
{
  auto cache = Path2dCalcCache(path);
  auto island = vector<Entity>();

  for (auto i = 0; i < count; i += 1)
  {
    auto t = cache.calcNormalizedTime(i / (count - 1.0f));
    auto p = path.getPosition(t);
    auto tangent = normalize(path.getTangent(t));
    auto normal = vec2(-tangent.y, tangent.x);

    auto pos = p;
    island.emplace_back( createShrub(entities, pos) );
    island.emplace_back( createShrub(entities, pos + normal * randFloat(2.0f)) );
    island.emplace_back( createShrub(entities, pos - normal * randFloat(2.0f)) );
  }

  for (auto e : island)
  {
    e.assign<Island>( island_id );
  }
  return island;
}

std::vector<entityx::Entity> gatherIsland(entityx::EntityManager &entities, uint32_t island)
{
  vector<Entity> members;
  ComponentHandle<Island> isle;
  for (auto e : entities.entities_with_components(isle))
  {
    if (isle->_id == island)
    {
      members.push_back(e);
    }
  }
  return members;
}

void mapIslandToPath(const std::vector<entityx::Entity> &entities, const ci::Path2d &path)
{
  auto cache = Path2dCalcCache(path);
  auto i = 0.0f;
  const auto length = entities.size() - 1.0f;
  auto offset = -1.0f;
  auto step_dir = 1;

  for (auto e : entities)
  {
    auto t_offset = mix(0.0f, 0.5f, i / length);
    auto t_desired = 0.5f + t_offset * step_dir;
    auto time = i / length;
    step_dir *= -1;

    auto t = cache.calcNormalizedTime(t_desired);

    i += 1.0f;
    auto delay_t = time + randFloat(-0.2f, 0.2f);
    auto delay = mix(0.0f, 1.0f, easeOutQuad(glm::clamp(delay_t, 0.0f, 1.0f)));

    auto pos = path.getPosition(t);
    auto tangent = normalize(path.getTangent(t));
    auto normal = vec2(-tangent.y, tangent.x);

    auto end_pos = planar(pos + (offset * normal * randFloat(0.4f, 1.0f)));
//    auto vertical_offset = vec3(0, glm::simplex(pos * 0.1f) * 0.2f, 0);
    auto delta = distance(e.component<Transform>()->position(), end_pos);
    auto duration = lmap(glm::clamp(delta, 0.0f, 40.0f), 0.0f, 40.0f, 1.0f, 3.0f);
    sharedTimeline().apply(positionAnim(e))
      .hold(delay)
      .then<RampTo>(end_pos, duration, EaseInOutCubic());

    auto wind = e.component<WindReceiver>();
    if (wind)
    {
      sharedTimeline().apply(&wind->_influence)
        .hold(delay)
        .set(0.0f)
        .holdUntil(delay + duration)
        .then<RampTo>(1.0f, 1.5f);
    }

    if(offset == -1.0f)
    {
      offset = 0.1f;
    }
    else if(offset == 1.0f)
    {
      offset = -1.0f;
    }
    else
    {
      offset = 1.0f;
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
