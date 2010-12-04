/*
 *  Particle.h
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
#include "Tile.h"

class Tile;

class Particle
{
public:
	Vec2f pos, vel;
	Tile *tile;
	float expired, lifetime;
	Rand *rand;
	
	vector<Tile*> *tiles;
	
	Particle(vector<Tile*> *_tiles, Tile *_tile, Vec2f _pos, float _lifetime, Rand* _r);
	
	void update(float dt);
	
	void draw(float scale);
};