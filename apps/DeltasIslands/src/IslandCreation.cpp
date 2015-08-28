//
//  IslandCreation.cpp
//
//  Created by David Wicks on 8/28/15.
//
//

#include "IslandCreation.h"
#include "Transform.h"
#include "InstanceShape.h"

#include "cinder/Rand.h"

using namespace cinder;
using namespace entityx;
using namespace std;

namespace sansumbrella
{

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

Entity createShrub(entityx::EntityManager &entities, const ci::vec2 &pos)
{
  auto e = entities.create();
  e.assign<Transform>( vec3(pos.x, 0.0f, pos.y) );
  e.assign<InstanceShape>( randFloat() );

  return e;
}

} // namespace sansumbrella
