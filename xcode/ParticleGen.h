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
	Tile *origin;
	vector<Particle*> particles;
	float ownLifetime, interval, lifetime, acc, ownExpired;
	Vec2f pos;
	Rand *rand;
	gl::Texture *texture, *texture_inv;
	vector<Tile*> *tiles;
	
	ParticleGen(vector<Tile*> *_tiles, Tile* _tile, Tile* _origin, float _olt, float _iv, float _lt, gl::Texture* _texture = NULL, gl::Texture* _texture_inv = NULL);
	~ParticleGen();
	
	void update(float dt);
	void draw();
};