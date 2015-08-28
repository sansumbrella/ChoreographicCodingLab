//
//  ExpiresSystem.h
//

//
//

#pragma once

#include "entityx/System.h"

namespace sansumbrella {

class ExpiresSystem : public entityx::System<ExpiresSystem>
{
public:
	void update( entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt ) override;

private:

};

} // namespace sansumbrella
