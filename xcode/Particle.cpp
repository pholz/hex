/*
 *  Particle.cpp
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Particle.h"
	
Particle::Particle(vector<Tile*> *_tiles, Tile *_tile, Vec2f _pos, float _lifetime, Rand* _r)
{
	tiles = _tiles;
	tile = _tile;
	lifetime = _lifetime;
	pos = _pos;
	expired = .0f;
	rand = _r;
	vel = Vec2f(rand->nextFloat(-PSPEED, PSPEED), rand->nextFloat(-PSPEED, PSPEED));
}

void Particle::update(float dt)
{
	Vec2f newpos = pos + vel;
	
	if(insidePolygon(newpos - tile->pos, (*tile->hex), tile->scale))
	{
		pos = newpos;
		
	}
	else 
	{
		Vec2f newpos2 = pos + vel * 15.0f;
		
		Vec2f exitdir = newpos2 - tile->pos;
		
		bool wander = false;
		vector<Tile*>::iterator tileit;
		
		int idx = Tile::getIndexForAngle(math<float>::atan2(exitdir.x, exitdir.y));
		console() << idx << endl;
		
		for(tileit = tiles->begin(); tileit < tiles->end(); tileit++)
		{
			if(insidePolygon(newpos2 - (*tileit)->pos, *(*tileit)->hex, (*tileit)->scale) &&
			   tile->connections[idx] &&
			   !tile->state[idx] &&
			   !(*tileit)->state[(idx+3) % 6])
			{
				pos = newpos;
				wander = true;
				tile = tile->connections[idx];
			}
		}
		
		if(!wander)
		{
			Rand r;
			vel = Vec2f(rand->nextFloat(-PSPEED, PSPEED), rand->nextFloat(-PSPEED, PSPEED));
		}
	}
	
	
	expired += dt;
}

void Particle::draw(float scale)
{
	glPushMatrix();
	
	gl::translate(Vec3f(pos, .8f));
	gl::rotate(-90 + toDegrees(math<float>::atan2(vel.normalized().y, vel.normalized().x)));
	gl::scale(Vec3f(scale, scale, 1.0f));
//	gl::color(ColorA(.2f, .2f, .2f, 1.0f));

	//gl::drawSolidRect(Rectf(-10, -5, 10, 5));
	
	//glDisable(GL_BLEND);
	gl::color(ColorA(.3f, .3f, .3f, 1.0f));
//	gl::drawSolidCircle(Vec2f(.0f, .0f), 4.0f, 16);
//	gl::drawSolidRect(Rectf(2, -2, 15, 2));
	
	glLineWidth(4.0f);
	gl::drawLine(Vec2f(-7.0f, -3.0f), Vec2f(.0f, .0f));
	gl::drawLine(Vec2f(7.0f, -3.0f), Vec2f(.0f, .0f));
	
	gl::translate(Vec2f(0.0f, -5.0f));
	gl::drawLine(Vec2f(-7.0f, -3.0f), Vec2f(.0f, .0f));
	gl::drawLine(Vec2f(7.0f, -3.0f), Vec2f(.0f, .0f));
	
//	glEnable(GL_BLEND);
	
	
	glPopMatrix();
	
}



