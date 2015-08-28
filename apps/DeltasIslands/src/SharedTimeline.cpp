//
//  SharedTimeline.cpp
//
//  Created by Soso Limited on 7/21/15.
//
//

#include "SharedTimeline.h"

using namespace soso;

ch::Timeline& soso::sharedTimeline()
{
	static auto timeline = ch::Timeline();

	return timeline;
}
