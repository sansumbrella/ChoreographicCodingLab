//
//  IslandCreation.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once

#include "entityx/Entity.h"
#include "cinder/Path2d.h"

namespace sansumbrella
{

/// Make a new island from a path.
std::vector<entityx::Entity> createIslandFromPath(entityx::EntityManager &entities, const ci::Path2d &path, uint32_t island_id=0, int count=100);

/// Returns a list of all the entities belonging to the given island.
std::vector<entityx::Entity> gatherIsland(entityx::EntityManager &entities, uint32_t island);

/// Map an existing island to a new path.
void mapIslandToPath(const std::vector<entityx::Entity> &entities, const ci::Path2d &path, float drawing_duration);

void animateIslandIntoPosition(const std::vector<entityx::Entity> &entities, float drawing_duration);

/*
/// TODO: use 3d polylines for creating islands
std::vector<entityx::Entity> createIslandFromLine(entityx::EntityManager &entities, const ci::PolyLine3f &path);
void mapIslandToLine(const std::vector<entityx::Entity> &entities, const ci::Path2d &path);
*/

/// Creates a single instanced shape.
entityx::Entity createShrub(entityx::EntityManager &entities, const ci::vec2 &pos);

/// Creates a curve through a random walk.
ci::Path2d randomPath();

ci::Path2d pathThroughPoints(const std::vector<ci::vec2> &points);

} // namespace sansumbrella
