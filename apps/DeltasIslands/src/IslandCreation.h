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
std::vector<entityx::Entity> createIslandFromPath(entityx::EntityManager &entities, const ci::Path2d &path);

/// Map an existing island to a new path.
void mapIslandToPath(const std::vector<entityx::Entity> &entities, const ci::Path2d &path);

entityx::Entity createShrub(entityx::EntityManager &entities, const ci::vec2 &pos);

ci::Path2d randomPath();

} // namespace sansumbrella
