/*
 *  Tile.cpp
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "Tile.h"
#include <sstream>
#include "cinder/Text.h"

Tile::Tile(int _id, Vec2f _pos, float _z, float _phi, float _scale, PolyLine<Vec2f>* _hex, int *_state)
{
	pos = _pos;
	phi = _phi;
	id = _id;
	z = _z;
	
	selectedCorner = -1;
	highlighted = selected = false;
	
	rx = ry = 0;
	
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
	
	pulseSpeed = .05f;
	pulseCounter = .0f;
	brightness = .0f;
}

Tile::~Tile()
{
	delete hex;
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

void Tile::draw()
{
	glLineWidth(1.0f);
	glPushMatrix();
	
	gl::translate(Vec3f(pos.x, pos.y, z));
	
	glPushMatrix();
	
	gl::rotate(Vec3f(rx, ry, phi));
	gl::scale(Vec3f(scale, scale, 1.0f));
	
	gl::color(TILECOLOR);
	
	gl::draw(*hex);
	
	if(selected)
	{
		gl::color(Color(.5f, 1.0f, .5f));
	}
	else if(!pulseSpeed && !brightness)
		gl::color(TILECOLOR2);
	else if(brightness) // brightness from volume
	{
		gl::color(Color(brightness, brightness, brightness));
	}
	else // pulsing
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
	
	if(selectedCorner >= 0)
	{
		glPushMatrix();
		gl::translate(Vec3f(.0f, .0f, 0.1f));
		gl::color(Color(1.0f, .0f, .0f));
		gl::drawSolidCircle(hex->getPoints()[selectedCorner], 5.0f, 16);
		glPopMatrix();
	}
	
	glPopMatrix();
	
	if(selected)
	{
		
		gl::color(Color(1.0f, .0f, .0f));
		TextLayout simple;
		simple.setFont( Font( "Helvetica", 24 ) );
		simple.setColor( Color( 1.0f, 1.0f, 1.0f ) );
		stringstream ss;
		ss << id;
		simple.addLine( ss.str() );
		gl::Texture idtex( simple.render( true, false ) );
		
		gl::translate(Vec3f(- idtex.getWidth()/2.0f, - idtex.getHeight()/2.0f, .5f));
		
		gl::draw(idtex);
	}
	
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