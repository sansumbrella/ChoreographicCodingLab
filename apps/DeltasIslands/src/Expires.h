//
//  Expires.h
//

//
//

#pragma once

#include "entityx/Entity.h"

namespace sansumbrella {

struct Expires
{
	Expires() = default;

	explicit Expires(float time)
	: time(time)
	{}

	float																	time = 1.0f;
	std::function<void (entityx::Entity)>	last_wish;
};

} // namespace sansumbrella
