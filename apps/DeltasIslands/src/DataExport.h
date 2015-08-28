//
//  DataExport.h
//
//  Created by David Wicks on 8/28/15.
//
//

#pragma once
#include "entityx/Entity.h"

namespace sansumbrella
{

inline std::string serializationHeader() { return "px,py,pz,openness\n"; }
std::string serializePositions(entityx::EntityManager &entities);

} // namespace sansumbrella
