/*
 *  Tile.h
 *  hex
 *
 *  Created by Peter Holzkorn on 29.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "util.h"
#include <vector>
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "Particle.h"

class Particle;

class Tile
{
public:
	
	PolyLine<Vec2f>* hex;
	
	int id;
	
	Vec2f pos;
	float z;
	float phi, rx, ry;
	float scale;
	Tile* connections[6];
	int state[6];
	int selectedCorner;
	
	Vec2f item_pos;
	bool item_visible;
	gl::Texture *item;
	
	float pulseSpeed;
	float pulseCounter;
	float brightness;
	
	Tile(int _id, Vec2f _pos, float z, float _phi, float _scale, PolyLine<Vec2f>* _hex, int *_state);
	~Tile();
	static int getIndexForAngle(float angle);
	void setState(const int *_state);
	void update(float dt);
	void draw(vector<Particle*> &particles);
	void collide(Tile* tile);
	void connect(Tile* tile, int pos);
	void disconnect(Tile* tile);
	
	
};
