#include "util.h"
#include "cinder/gl/gl.h"
#include <vector>

#define TILERAD 50.0f
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
	
	Tile(Vec2f _pos, float _phi, PolyLine<Vec2f>* _hex)
	{
		pos = _pos;
		phi = _phi;
		
		hex = _hex;
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
		
		glPopMatrix();
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
	tiles->push_back(new Tile(Vec2f(200.0f, 200.0f), .0f, hex));
	tiles->push_back(new Tile(Vec2f(300.0f, 200.0f), .0f, hex));
	tiles->push_back(new Tile(Vec2f(400.0f, 200.0f), .0f, hex));
	tiles->push_back(new Tile(Vec2f(500.0f, 200.0f), .0f, hex));
	
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
