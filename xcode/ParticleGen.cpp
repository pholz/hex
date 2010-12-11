/*
 *  ParticleGen.cpp
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ParticleGen.h"

ParticleGen::ParticleGen(vector<Tile*> *_tiles, Tile* _tile, float _iv, float _lt, gl::Texture* _texture)
{
	texture = _texture;
	tiles = _tiles;
	pos = _tile->pos;
	tile = _tile;
	interval = _iv;
	lifetime = _lt;
	acc = .0f;
	
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
	
	if(acc > interval)
	{
		acc = .0f;
		particles.push_back(new Particle(tiles, tile, pos, lifetime, rand, texture));
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