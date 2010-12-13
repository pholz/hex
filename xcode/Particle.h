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
	Vec2f pos, vel, bounce_targetvel;
	Tile *tile;
	Tile* origin;
	float expired, lifetime;
	Rand *rand;
	ParticleState state;
	float alpha;
	static vector<Particle*>* remains;
	
	
	float bounce_rotationstep, bounce_rotationtime;
	float lastarc;
	float dying_time;
	
	gl::Texture *texture, *texture_inv, *texture_dead;
	
	vector<Tile*> *tiles;
	
	Particle(vector<Tile*> *_tiles, Tile *_tile, Tile *_origin, Vec2f _pos, float _lifetime, Rand* _r, gl::Texture* _texture = NULL, gl::Texture* _texture_inv = NULL);
	
	void update(float dt);
	void setState(ParticleState newstate, bool override = false);
	void draw(float scale);
};


