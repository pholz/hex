/*
 *  Tile.cpp
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "Tile.h"

Tile::Tile(int _id, Vec2f _pos, float _z, float _phi, float _scale, PolyLine<Vec2f>* _hex, int *_state)
{
	pos = _pos;
	phi = _phi;
	id = _id;
	z = _z;
	
	hex = _hex;
	scale = _scale;
	
	item_pos = Vec2f(.0f, .0f);
	item_visible = false;
	item = NULL;
	
	//	scale = 1.0f;
	
	for(int i = 0; i< 6; i++)
	{
		connections[i] = 0;
		state[i] = (i == 1 ? 1 : 0);
	}
	
	setState(_state);
	
	pulseSpeed = .0f;
	pulseCounter = .0f;
}

int Tile::getIndexForAngle(float angle)
{
	//return (int) ((-angle-M_PI/2.0f) / (M_PI/3.0f));
	return (int( (angle + M_PI - M_PI/6.0f) /(M_PI/3.0f)) + 1) % 6;
}

void Tile::setState(const int *_state)
{
	memcpy((void*)&state, (void*)_state, 6 * sizeof(int));
}

void Tile::update(float dt)
{
	if(pulseSpeed > .0f)
	{
		pulseCounter += dt * pulseSpeed * M_PI;
	}
}

void Tile::draw(vector<Particle*> &particles)
{
	glLineWidth(1.0f);
	glPushMatrix();
	
	// fill the alpha component with 1
	glDisable(GL_BLEND);
	gl::disableDepthWrite();
	gl::disableDepthRead();
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	
	glColor4f(0, 0, 0, 1);
	gl::drawSolidRect(Rectf(0, 0, WIDTH, HEIGHT));
	
	glEnable(GL_BLEND);
	gl::enableDepthWrite(true);
	gl::enableDepthRead(true);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	
	gl::translate(Vec3f(pos.x, pos.y, z));
	
	glPushMatrix();
	
	gl::rotate(phi);
	gl::scale(Vec3f(scale, scale, 1.0f));
	
	gl::color(TILECOLOR);
	
	gl::draw(*hex);
	
	if(!pulseSpeed)
		gl::color(TILECOLOR2);
	else
	{
		float b = math<float>::abs(math<float>::sin(pulseCounter));
		gl::color(Color(b, b, b));
	}
		
	
	glBegin(GL_TRIANGLE_FAN);
	
	PolyLine<Vec2f>::iterator pt;
	
	for(pt = hex->begin(); pt < hex->end(); pt++)
	{
		gl::vertex(*pt);
	}
	
	glEnd();
	
	// write 0 in the alpha component where our hex is
	glDisable(GL_BLEND);
	gl::disableDepthWrite();
	gl::disableDepthRead();
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	
	glColor4f(0, 0, 0, 1);
	gl::drawSolidRect(Rectf(0, 0, WIDTH, HEIGHT));
	
	glColor4f(1, 1, 1, 0);
	
	glBegin(GL_TRIANGLE_FAN);
	
	for(pt = hex->begin(); pt < hex->end(); pt++)
	{
		gl::vertex(*pt);
	}
	
	glEnd();
	
	// render masked layer on top; where it's zero, set src alpha to 1; set dst alpha to whatever it already is
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_BLEND);
	gl::enableDepthWrite(true);
	gl::enableDepthRead(true);
	//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
	glBlendFunc (GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
	
	//gl::color(Color(1.0f, .0f, .0f));
	//gl::drawSolidCircle(Vec2f(20.0f, 20.0f), 60.0f, 16);
	
	glPopMatrix();
	
	if(true) 
	{
		glPushMatrix();
		
		
		//gl::color(Color(.0f, .0f, .0f));
		//gl::drawSolidRect(Rectf(0, 0, 10, 80));
		
		vector<Particle*>::iterator it;
		for(it = particles.begin(); it < particles.end(); it++)
		{
			if(insidePolygon( (*it)->pos - pos, *hex, scale ))
			{
				glPushMatrix();
				gl::translate(Vec3f((*it)->pos - pos, z+.05f));
				(*it)->draw(scale);
				glPopMatrix();
			}
		}
		
		glPopMatrix();
		
	}
	
	// return to normal alpha blending
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glPopMatrix();
}

void Tile::collide(Tile* tile)
{
	Vec2f vec = tile->pos - pos;
	float rads = math<float>::atan2(vec.x, vec.y) + M_PI;
	
	if(vec.length() < 2 * TILERAD_MIN + 5.0f && vec.length() > 2 * TILERAD_MIN - 5.0f)
	{
		//
		
		for(int i = 0; i < 6; i++)
		{
			if(rads < i * M_PI/3.0f + M_PI/24.0f && rads > i * M_PI/3.0f - M_PI/24.0f)
			{
				//console() << toDegrees(rads) << endl;
				connect(tile, i);
				tile->connect(this, (3 + i) % 6);
			}
		}
	}
	else
	{
		disconnect(tile);
		tile->disconnect(this);
	}
	
	
}

void Tile::connect(Tile* tile, int pos)
{
	connections[pos] = tile;
}

void Tile::disconnect(Tile* tile)
{
	for(int i = 0; i < 6; i++)
	{
		if(connections[i] && connections[i] == tile)
			connections[i] = 0;
	}
}