/*
 *  util.h
 *  hex
 *
 *  Created by Peter Holzkorn on 18.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "cinder/app/AppBasic.h"
#include "cinder/CinderMath.h"
#include "cinder/rand.h"

#define TILERAD 50.0f
#define TILERAD_MIN math<float>::cos(M_PI/6.0f) * TILERAD
#define TILECOLOR Color(1.0f, 1.0f, 1.0f)
#define TILECOLOR2 Color(.3f, .3f, .3f)
#define BLACK Color(.0f, .0f, .0f)
#define PSPEED 1.0f
#define WIDTH 1600
#define HEIGHT 1050

using namespace ci;
using namespace ci::app;
using namespace std;

typedef struct {
	float r;
	float theta;
} polarCoords;

enum ParticleState {
	FADEIN,
	MOVING,
	BOUNCING,
	FADEOUT,
	DYING,
	DEAD,
	OVERSEA
};


Vec2f cart(float r, float theta);
polarCoords polar(Vec2f v);
bool insidePolygon(Vec2f p, PolyLine<Vec2f>& poly, float scale);
float cint(float x);