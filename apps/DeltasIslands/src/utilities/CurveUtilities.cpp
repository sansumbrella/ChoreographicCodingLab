/*
 * Copyright (c) 2013, David Wicks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "CurveUtilities.h"
#include "cinder/Vector.h"

using namespace cinder;
using namespace std;

template <typename T>
vector<T> pockets::curveThrough(const vector<T> &points)
{
	assert( points.size() > 2 );
	vector<T> curvePoints;

	T p1 = points.at( 0 );
	T p2 = points.at( 1 );
	T p3 = points.at( 2 );

	T b1 = p1 + (p2 - p1) / 6.0f;
	T b2 = p2 - (p3 - p1) / 6.0f;

	curvePoints.push_back( p1 );
	curvePoints.push_back( b1 );
	curvePoints.push_back( b2 );
	curvePoints.push_back( p2 );

	for( int i = 1; i < points.size() - 2; ++i )
	{
		T p0 = points.at( i - 1 );
		T p1 = points.at( i );
		T p2 = points.at( i + 1 );
		T p3 = points.at( i + 2 );

		T b1 = p1 + (p2 - p0) / 6.0f;
		T b2 = p2 - (p3 - p1) / 6.0f;

		curvePoints.push_back( p1 );
		curvePoints.push_back( b1 );
		curvePoints.push_back( b2 );
		curvePoints.push_back( p2 );
	}

	size_t i2 = points.size() - 1;
	size_t i1 = i2 - 1;
	size_t i0 = i2 - 2;
	T p0 = points.at( i0 );
	p1 = points.at( i1 );
	p2 = points.at( i2 );

	b1 = p1 + (p2 - p0) / 6.0f;
	b2 = p2 - (p2 - p1) / 6.0f;
	curvePoints.push_back( p1 );
	curvePoints.push_back( b1 );
	curvePoints.push_back( b2 );
	curvePoints.push_back( p2 );

	return curvePoints;
}

template vector<vec2> pockets::curveThrough<vec2>(const vector<vec2> &points);
template vector<vec3> pockets::curveThrough<vec3>(const vector<vec3> &points);
