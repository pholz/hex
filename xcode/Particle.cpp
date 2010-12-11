/*
 *  Particle.cpp
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Particle.h"
	
Particle::Particle(vector<Tile*> *_tiles, Tile *_tile, Tile *_origin, Vec2f _pos, float _lifetime, Rand* _r, gl::Texture * _texture)
{
	tiles = _tiles;
	tile = _tile;
	origin = _origin;
	lifetime = _lifetime;
	texture = _texture;
	pos = _pos;
	expired = .0f;
	alpha = .0f;
	lastarc = .0f;
	dying_time = .0f;
	bounce_rotationstep = bounce_rotationtime = .0f;
	rand = _r;
	setState(FADEIN);
	vel = Vec2f(rand->nextFloat(-PSPEED, PSPEED), rand->nextFloat(-PSPEED, PSPEED));
}

void Particle::setState(ParticleState newstate, bool override)
{
	if( (state != DYING && state != BOUNCING) || override == true)
	{
		state = newstate;
	}
}

void Particle::update(float dt)
{
	if(state == FADEIN)
	{
		alpha = expired / 2.0f;
		
		if(expired >= 2.0f)
		{
			setState(MOVING);
		}
			
	}
	
	if(state == MOVING || state == FADEIN || state == FADEOUT)
	{

		if(rand->nextInt(100) > 90)
		{
			lastarc = rand->nextFloat(-.01f, .01f);
		}
		vel.rotate(lastarc);
		Vec2f newpos = pos + vel;
		
		// check if we're still inside
		if(insidePolygon(newpos - tile->pos, (*tile->hex), tile->scale))
		{
			pos = newpos;
			
		}
		else // move on to next tile or bounce off the wall
		{
			Vec2f newpos2 = pos + vel * 15.0f;
			
			bool wander = false;
			
			vector<Tile*>::iterator tileit;
			for(tileit = tiles->begin(); tileit < tiles->end(); tileit++)
			{
				if(insidePolygon(newpos2 - (*tileit)->pos, *(*tileit)->hex, (*tileit)->scale))
				{
					pos = newpos;
					wander = true;
					tile = *tileit;
				}
			}
			
			if(!wander)
			{
				setState(BOUNCING);
				bounce_targetvel = Vec2f(rand->nextFloat(-PSPEED, PSPEED), rand->nextFloat(-PSPEED, PSPEED));
				polarCoords target = polar(bounce_targetvel);
				polarCoords current = polar(vel);
				
				bounce_rotationstep = ( target.theta - current.theta) * 4.0f;
				bounce_rotationtime = .25f;
			}
		}
		
	}
	
	if(state == BOUNCING)
	{
		
		vel.rotate(bounce_rotationstep * dt);
		bounce_rotationtime -= dt;
		if(bounce_rotationtime <= .0f)
			setState(MOVING, true);
		
	}
	
	if(expired >= lifetime - 4.0f)
	{
		setState(FADEOUT);
	}
	
	if(state == FADEOUT)
	{
		alpha = (lifetime - expired) / 4.0f;
	}
	
	if(state == DYING)
	{
		dying_time += dt;
		
		if(dying_time > 3.0f)
		{
			expired = lifetime;
		}
		
	}
	
	
	expired += dt;
}

void Particle::draw(float scale)
{
	glPushMatrix();
	
	gl::translate(Vec3f(pos, .8f));
	gl::rotate(90 + toDegrees(math<float>::atan2(vel.normalized().y, vel.normalized().x)));
	gl::scale(Vec3f(scale, scale, 1.0f));
//	gl::color(ColorA(.2f, .2f, .2f, 1.0f));

	//gl::drawSolidRect(Rectf(-10, -5, 10, 5));
	
	//glDisable(GL_BLEND);
	gl::color(ColorA(.3f, .3f, .3f, alpha));
//	gl::drawSolidCircle(Vec2f(.0f, .0f), 4.0f, 16);
//	gl::drawSolidRect(Rectf(2, -2, 15, 2));
	
	if(!texture){
		glLineWidth(4.0f);
		gl::drawLine(Vec2f(-7.0f, -3.0f), Vec2f(.0f, .0f));
		gl::drawLine(Vec2f(7.0f, -3.0f), Vec2f(.0f, .0f));
		
		gl::translate(Vec2f(0.0f, -5.0f));
		gl::drawLine(Vec2f(-7.0f, -3.0f), Vec2f(.0f, .0f));
		gl::drawLine(Vec2f(7.0f, -3.0f), Vec2f(.0f, .0f));
	} else if(state != DYING) {
		gl::enableAlphaBlending();
		gl::disableDepthWrite();
		//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gl::scale(Vec3f(.05f, .05f, 1.0f));
		gl::translate(Vec2f(-texture->getWidth(), -texture->getHeight()));
		gl::draw(*texture);
		
		gl::enableDepthWrite();
	} else {
		gl::enableAlphaBlending();
		gl::disableDepthWrite();
		gl::color(ColorA(1.0f, .0f, .0f, 1.0f-dying_time/3.0f));
		gl::drawSolidCircle(Vec2f(.0f,.0f), 10.0f-dying_time*2.0f, 32);
	}
	
	
	
//	glEnable(GL_BLEND);
	
	
	glPopMatrix();
	
}



