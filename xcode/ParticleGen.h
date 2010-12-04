/*
 *  ParticleGen.h
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "Particle.h"

class ParticleGen
{
public:
	Tile *tile;
	vector<Particle*> particles;
	float interval, lifetime, acc;
	Vec2f pos;
	Rand *rand;
	vector<Tile*> *tiles;
	
	ParticleGen(vector<Tile*> *_tiles, Tile* _tile, float _iv, float _lt);
	
	void update(float dt);
	
	void draw();
};