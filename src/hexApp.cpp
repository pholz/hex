#include "ParticleGen.h"
#include "cinder/ImageIo.h"
#include <iostream>
#include <fstream>
#include "boost/algorithm/string.hpp"
#include "cinder/Filesystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class hexApp : public AppBasic {

	vector< Tile* > *tiles;
	Tile* dragging;
	Vec2f dragoffset;
	ParticleGen *pgen;
	float last;
	//ofstream file_out;
	//ifstream file_in;
	
	Surface surf_tank;
	
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings(Settings* settings);
	void keyDown( KeyEvent event );
};

void hexApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(WIDTH, HEIGHT);
	//settings->setFullScreen(true);
	
	//file_out = ofstream("params.txt");
}

void hexApp::setup()
{
	surf_tank = Surface(loadImage(loadResource("tank.png")));
	
	PolyLine<Vec2f> * hex = new PolyLine<Vec2f>();
	hex->setClosed(true);
	
	for(int i = 0; i < 6; i++)
	{
		hex->push_back(cart(TILERAD, (float)i * M_PI/3.0f));
	}
	
	
	tiles = new vector< Tile* >();
	
	int s0[] = {1, 0, 0, 0, 0, 0};
	
	// read tile pos/phi/scale params from file
	
	ifstream file_in("params.txt");
	
	while(!file_in.eof())
	{
		char buf[100];
		file_in.getline(buf, 100);
		vector<string> strs;
		boost::algorithm::split(strs, buf, boost::algorithm::is_any_of(","));
		
		if(strs.size() < 5) break;
		
		tiles->push_back(new Tile((int)atoi(strs[0].c_str()), Vec2f(strtod(strs[1].c_str(), NULL), strtod(strs[2].c_str(), NULL)), 
								  strtod(strs[3].c_str(), NULL), strtod(strs[4].c_str(), NULL), hex, s0));
		
	}
	
	dragging = 0;
	
	pgen = new ParticleGen(tiles, (*tiles)[1], 2.0f, 15.0f);
	
	last = getElapsedSeconds();
	
	(*tiles)[3]->item = new gl::Texture(surf_tank);
}

void hexApp::keyDown( KeyEvent event )
{
	if(dragging && event.getChar() == 'a')
	{
		dragging->phi-=.5f;
	}
	else if(dragging && event.getChar() == 'd')
	{
		dragging->phi+=.5f;
	}
	if(dragging && event.getChar() == 'w')
	{
		dragging->scale+=.01f;
	}
	if(dragging && event.getChar() == 's')
	{
		dragging->scale-=.01f;
	}
	
	
	if(event.getChar() == 'p')
	{
		ofstream file_out("params.txt");
		
		vector<Tile*>::iterator tile;
		for(tile = tiles->begin(); tile < tiles->end(); tile++)
		{
			Tile &t = *(*tile);
			
			file_out << t.id << "," << t.pos.x << "," << t.pos.y << "," << t.phi << "," << t.scale << endl; 
			
		}
		
		file_out.close();
		
		
	}
	 
}

void hexApp::mouseDown( MouseEvent event )
{
	Vec2f mpos = Vec2f(event.getX(), event.getY());
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		Vec2f collpos = mpos - (*tile)->pos;
		if(insidePolygon(collpos, (*(*tile)->hex), (*tile)->scale))
		{
			//console() << "dragging on" << endl;
			dragoffset = collpos;
			dragging = (*tile);
		}
	}
}

void hexApp::mouseUp( MouseEvent event )
{
	dragging = 0;
	//console() << "dragging off" << endl;
}

void hexApp::mouseDrag( MouseEvent event )
{
	if(dragging)
	{
		//console() << "dragging" << endl;
		dragging->pos = Vec2f(event.getX(), event.getY()) - dragoffset;;
	}
}

void hexApp::update()
{
	float now = getElapsedSeconds();
	float dt = now - last;
	last = now;
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		vector<Tile*>::iterator tile2;
		for(tile2 = tile; tile2 < tiles->end(); tile2++)
		{
			if(tile != tile2)
				(*tile)->collide(*tile2);
		}
	}
	pgen->update(dt);
	
	(*tiles)[3]->item_pos = Vec2f(math<float>::sin(getElapsedSeconds()) * 20.0f, .0f);
}

void hexApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::enableAlphaBlending();
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		(*tile)->draw(pgen->particles);
	}
	
	
	
//	pgen->draw();
	
	//gl::draw(gl::Texture(surf_tank), getWindowBounds());
	
}


CINDER_APP_BASIC( hexApp, RendererGl )
