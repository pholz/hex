/*
 *  ParticleGen.cpp
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ParticleGen.h"

ParticleGen::ParticleGen(vector<Tile*> *_tiles, Tile* _tile, Tile* _origin, float _olt, float _iv, float _lt, gl::Texture* _texture, gl::Texture* _texture_inv)
{
	texture = _texture;
	texture_inv = _texture_inv;
	tiles = _tiles;
	pos = _tile->pos;
	tile = _tile;
	origin = _origin;
	interval = _iv;
	lifetime = _lt;
	ownLifetime = _olt;
	acc = .0f;
	ownExpired = .0f;
	
	rand = new Rand();
}

ParticleGen::~ParticleGen()
{
	particles.clear();
	delete rand;
}

void ParticleGen::update(float dt)
{
	acc += dt;
	ownExpired += dt;
	
	
	
	if( (ownExpired < ownLifetime - lifetime) && acc > interval)
	{
		acc = .0f;
		particles.push_back(new Particle(tiles, tile, origin, pos, lifetime, rand, texture, texture_inv));
	}
	
	vector<Particle*>::iterator it;
	for(it = particles.begin(); it < particles.end(); it++)
	{
		if((*it)->expired > (*it)->lifetime)
		{
			particles.erase(it);
			continue;
		}
		
		(*it)->update(dt);
	}
		
	
}

void ParticleGen::draw()
{
	vector<Particle*>::iterator it;
	for(it = particles.begin(); it < particles.end(); it++)
	{
		(*it)->draw(1.0f);
	}
}