#include "util.h"
#include "cinder/gl/gl.h"
#include <vector>
#include "cinder/Rand.h"

#define TILERAD 50.0f
#define TILERAD_MIN math<float>::cos(M_PI/6.0f) * TILERAD
#define TILECOLOR Color(1.0f, 1.0f, 1.0f)
#define TILECOLOR2 Color(.5f, .5f, .5f)

using namespace ci;
using namespace ci::app;
using namespace std;

class Tile
{
public:
	
	PolyLine<Vec2f>* hex;
	Vec2f pos;
	float phi;
	Tile* connections[6];
	int state[6];
	
	
	
	Tile(Vec2f _pos, float _phi, PolyLine<Vec2f>* _hex, int *_state)
	{
		pos = _pos;
		phi = _phi;
		
		hex = _hex;
		
		Rand r;
		
		for(int i = 0; i< 6; i++)
		{
			connections[i] = 0;
			state[i] = (i == 1 ? 1 : 0);
		}
			
		setState(_state);
	}
	
	void setState(const int *_state)
	{
		memcpy((void*)&state, (void*)_state, 6 * sizeof(int));
	}
	
	void draw()
	{
		glPushMatrix();
		
		gl::translate(pos);
		
		gl::color(TILECOLOR);
		
		gl::draw(*hex);
		
		gl::color(TILECOLOR2);
		
		glBegin(GL_TRIANGLE_FAN);
		
		PolyLine<Vec2f>::iterator pt;

		for(pt = hex->begin(); pt < hex->end(); pt++)
		{
			gl::vertex(*pt);
		}
		
		glEnd();
		
		/*
		gl::color(Color(1.0f, .0f, .0f));
		for(pt = hex->begin(); pt < hex->end() - 1; pt++)
		{
			gl::vertex(*pt);
		}
		 */
		
		for(int i = 0; i < 6; i++)
		{
			if(connections[i])
			{
				Vec2f v = (connections[i]->pos - pos)/2.0f;
				gl::color(Color(.0f, 1.0f, .0f));
				gl::drawLine(Vec2f(.0f, .0f), v);
			}
			
			if(state[i])
			{
				Vec2f p = cart(TILERAD_MIN - 5.0f, -M_PI/2 - i * M_PI/3.0f);
				gl::color(Color(1.0f, .0f, .0f));
				gl::drawSolidCircle(p, 5.0f, 32);
			}
		}
		
		glPopMatrix();
	}
	
	void collide(Tile* tile)
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
					console() << toDegrees(rads) << endl;
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
	
	void connect(Tile* tile, int pos)
	{
		connections[pos] = tile;
	}
	
	void disconnect(Tile* tile)
	{
		for(int i = 0; i < 6; i++)
		{
			if(connections[i] && connections[i] == tile)
				connections[i] = 0;
		}
	}
};

class hexApp : public AppBasic {

	vector< Tile* > *tiles;
	Tile* dragging;
	Vec2f dragoffset;
	
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings(Settings* settings);
};

void hexApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(1024, 768);
}

void hexApp::setup()
{
	PolyLine<Vec2f> * hex = new PolyLine<Vec2f>();
	hex->setClosed(true);
	
	for(int i = 0; i < 6; i++)
	{
		hex->push_back(cart(TILERAD, (float)i * M_PI/3.0f));
	}
	
	
	tiles = new vector< Tile* >();
	
	int s0[] = {1, 0, 0, 0, 0, 0};
	int s1[] = {0, 1, 1, 0, 0, 0};
	int s2[] = {0, 0, 0, 1, 1, 0};
	int s3[] = {1, 0, 0, 0, 0, 1};
	tiles->push_back(new Tile(Vec2f(200.0f, 200.0f), .0f, hex, s0));
	tiles->push_back(new Tile(Vec2f(300.0f, 200.0f), .0f, hex, s1));
	tiles->push_back(new Tile(Vec2f(400.0f, 200.0f), .0f, hex, s2));
	tiles->push_back(new Tile(Vec2f(500.0f, 200.0f), .0f, hex, s3));
	
	dragging = 0;
}

void hexApp::mouseDown( MouseEvent event )
{
	Vec2f mpos = Vec2f(event.getX(), event.getY());
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		Vec2f collpos = mpos - (*tile)->pos;
		if(insidePolygon(collpos, (*(*tile)->hex)))
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
}

void hexApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		(*tile)->draw();
	}
}


CINDER_APP_BASIC( hexApp, RendererGl )
