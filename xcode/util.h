/*
 *  util.h
 *  hex
 *
 *  Created by Peter Holzkorn on 18.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/app/AppBasic.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef struct {
	float r;
	float theta;
} polarCoords;


Vec2f cart(float r, float theta)
{
	return Vec2f(r * math<float>::cos(theta), r * math<float>::sin(theta));
}

polarCoords polar(Vec2f v)
{
	polarCoords p;
	p.r = math<float>::sqrt(v.x*v.x + v.y*v.y);
	p.theta = math<float>::atan2(v.x, v.y);
	
	return p;
}

bool insidePolygon(Vec2f p, PolyLine<Vec2f>& poly)
{
	bool inside = false;
	for(int i = 0, j = poly.size()-1; i < poly.size(); j = i++)
	{
		Vec2f& p1 = poly.getPoints()[i];
		Vec2f& p2 = poly.getPoints()[j];
		
		if( ( p1.y <= p.y && p.y < p2.y || p2.y <= p.y && p.y < p1.y ) &&
		   p.x < (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x )
		{
			inside = !inside;
		}
	}
	
	return inside;
}