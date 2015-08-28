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

std::vector<entityx::Entity> createIslandFromPath(entityx::EntityManager &entities, const ci::Path2d &path);

entityx::Entity createShrub(entityx::EntityManager &entities, const ci::vec2 &pos);

} // namespace sansumbrella
