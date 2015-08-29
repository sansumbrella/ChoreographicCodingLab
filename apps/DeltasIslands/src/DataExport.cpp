//
//  DataExport.cpp
//
//  Created by David Wicks on 8/28/15.
//
//

#include "DataExport.h"
#include "Transform.h"
#include "InstanceShape.h"

using namespace sansumbrella;
using namespace entityx;

namespace sansumbrella
{

std::string serializePositions(entityx::EntityManager &entities)
{
  ComponentHandle<Transform>      xf;
  ComponentHandle<InstanceShape>  instance;
  std::stringstream ss;

  for (auto e : entities.entities_with_components(xf, instance))
  {
    ss << xf->position().x << "," << xf->position().y << "," << xf->position().z << "," << instance->_openness << std::endl;
  }
  
  return ss.str();
}

} // sansumbrella
