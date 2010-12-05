#include "ParticleGen.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include <iostream>
#include <fstream>
#include "boost/algorithm/string.hpp"
#include "cinder/Filesystem.h"
#include "OscListener.h"
#include "OscSender.h"

#define OSC_SEND_HOST "localhost"
#define OSC_SEND_PORT 5000
#define OSC_RECEIVE_PORT 3000

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
	CameraOrtho cam;
	int maxid;
	float zoom;
	float defaultfactors[6];
	Vec3f globalTranslate;
	
	osc::Listener listener;
	osc::Sender sender;
	
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings(Settings* settings);
	void keyDown( KeyEvent event );
	PolyLine<Vec2f>* genHex(float* factors);
	void shutdown();
	void oscUpdate();
};

void hexApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(WIDTH, HEIGHT);
	//settings->setFullScreen(true);
	
	//file_out = ofstream("params.txt");
}

PolyLine<Vec2f>* hexApp::genHex(float* factors)
{
	PolyLine<Vec2f> *hex = new PolyLine<Vec2f>();
	hex->setClosed(true);
	
	for(int i = 0; i < 6; i++)
	{
		hex->push_back(cart(TILERAD * factors[i], (float)i * M_PI/3.0f));
	}
	
	return hex;
}

void hexApp::setup()
{
	sender.setup(OSC_SEND_HOST, OSC_SEND_PORT);
	listener.setup(OSC_RECEIVE_PORT);
	
	globalTranslate = Vec3f(.0f, .0f, .0f);
	
	cam = CameraOrtho(0, getWindowWidth(), getWindowHeight(), 0, 0, 1000);
	//cam = CameraPersp( getWindowWidth(), getWindowHeight(), 50, 0.1, 10000 );
	zoom = 1000.0f;
	//cam.setWorldUp(Vec3f(.0f, 1.0f, .0f));
	
	surf_tank = Surface(loadImage(loadResource("tank.png")));
	
	float df[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	memcpy(defaultfactors, df, 6 * sizeof(float));
	
	
	tiles = new vector< Tile* >();
	
	int s0[] = {1, 0, 0, 0, 0, 0};
	
	// read tile pos/phi/scale params from file
	
	ifstream file_in("params.txt");
	
	maxid = 0;
	while(!file_in.eof())
	{
		char buf[100];
		file_in.getline(buf, 100);
		vector<string> strs;
		boost::algorithm::split(strs, buf, boost::algorithm::is_any_of(","));
		
		if(strs.size() < 5) break;
		
		if((int)atoi(strs[0].c_str()) > maxid) maxid = (int)atoi(strs[0].c_str());
		
		float facs[] = {strtod(strs[6].c_str(), NULL), strtod(strs[7].c_str(), NULL), strtod(strs[8].c_str(), NULL), strtod(strs[9].c_str(), NULL), strtod(strs[10].c_str(), NULL), strtod(strs[11].c_str(), NULL)};
		
		tiles->push_back(new Tile((int)atoi(strs[0].c_str()), //id
								  Vec2f(strtod(strs[1].c_str(), NULL), strtod(strs[2].c_str(), NULL)), //pos
								  strtod(strs[3].c_str(), NULL), //z
								  strtod(strs[4].c_str(), NULL), //phi
								  strtod(strs[5].c_str(), NULL), //scale
								  genHex(facs), 
								  s0));
		
	}
	
	dragging = 0;
	
	pgen = new ParticleGen(tiles, (*tiles)[1], 2.0f, 15.0f);
	
	last = getElapsedSeconds();
	
	//(*tiles)[3]->item = new gl::Texture(surf_tank);
}

void hexApp::oscUpdate()
{
	
	
	while (listener.hasWaitingMessages()) {
		osc::Message message;
		listener.getNextMessage(&message);
		
		string addr = message.getAddress();
		
		if(addr == "/max/vol"  && message.getNumArgs() == 1)
		{
			try {
				
				
				float vol = message.getArgAsFloat(0);
				if(dragging)
				{
					dragging->brightness = vol;
				}
								
			} catch (...) {
				console() << "Exception reading argument as float" << std::endl;
			}
		}
	}
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
	else if(dragging && event.getChar() == 'w')
	{
		dragging->scale+=.01f;
	}
	else if(dragging && event.getChar() == 's')
	{
		dragging->scale-=.01f;
	}
	else if(event.getChar() == 'l')
	{
		globalTranslate.x += 1.0f;
	}
	else if(event.getChar() == '\'')
	{
		globalTranslate.x -= 1.0f;
	}
	else if(event.getChar() == 'p')
	{
		globalTranslate.y += 1.0f;
	}
	else if(event.getChar() == ';')
	{
		globalTranslate.y -= 1.0f;
	}
	else if(dragging && event.getChar() == 'i')
	{
		dragging->ry-=.5f;
	}
	else if(dragging && event.getChar() == 'k')
	{
		dragging->ry+=.5f;
	}
	else if(dragging && event.getChar() == 'j')
	{
		dragging->rx-=.5f;
	}
	else if(dragging && event.getChar() == 'l')
	{
		dragging->rx+=.5f;
	}
	else if(dragging && event.getChar() == 'z')
	{
		dragging->z -= .05f;
	}
	else if(dragging && event.getChar() == 'x')
	{
		dragging->z += .05f;
	}
	else if(dragging && event.getChar() == '+')
	{
		dragging->pulseSpeed+=.1f;
	}
	else if(dragging && event.getChar() == '-')
	{
		dragging->pulseSpeed-=.1f;
	}
	else if(dragging && event.getChar() == 'c')
	{
		dragging->selectedCorner++;
		if(dragging->selectedCorner == 6) dragging->selectedCorner = -1;
	}
	else if(dragging && event.getChar() == '.' && dragging->selectedCorner >= 0)
	{
		dragging->hex->getPoints()[dragging->selectedCorner] += Vec2f( dragging->hex->getPoints()[dragging->selectedCorner].normalized() );
	}
	
	else if(dragging && event.getChar() == ',' && dragging->selectedCorner >= 0)
	{
		dragging->hex->getPoints()[dragging->selectedCorner] -= Vec2f( dragging->hex->getPoints()[dragging->selectedCorner].normalized() );
	}
	
	else if(event.getChar() == ']')
	{
		zoom -= 1.0f;
	}
	else if(event.getChar() == '[')
	{
		zoom += 1.0f;
	}
	
	else if(event.getChar() == 't')
	{
		int s0[] = {1, 0, 0, 0, 0, 0};
		tiles->push_back(new Tile(maxid+1, 
								  Vec2f(this->getMousePos()), 
								  .0f, //z
								  .0f, //phi
								  1.0f, //scale
								  genHex(defaultfactors), 
								  s0));
		++maxid;
	}
	
	
	else if(event.getChar() == 'p')
	{
		ofstream file_out("params.txt");
		
		vector<Tile*>::iterator tile;
		for(tile = tiles->begin(); tile < tiles->end(); tile++)
		{
			Tile &t = *(*tile);
			
			file_out << t.id << "," << t.pos.x << "," << t.pos.y << "," << t.z << "," << t.phi << "," << t.scale << ",";
			
			for(int i = 0; i < 6; i++)
			{
				polarCoords pc = polar(t.hex->getPoints()[i]);
				float factor = pc.r / TILERAD;
				file_out << factor;
				
				if(i < 5)
					file_out << ",";
				else
					file_out << endl;
				
			}
			
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
		(*tile)->update(dt);
		
		vector<Tile*>::iterator tile2;
		for(tile2 = tile; tile2 < tiles->end(); tile2++)
		{
			if(tile != tile2)
				(*tile)->collide(*tile2);
		}
	}
	
	oscUpdate();
	 
	//pgen->update(dt);
	
	(*tiles)[3]->item_pos = Vec2f(math<float>::sin(getElapsedSeconds()) * 20.0f, .0f);
}

void hexApp::draw()
{
	//cam.lookAt(Vec3f(getWindowWidth()/2, getWindowHeight()/2, -zoom), Vec3f(getWindowWidth()/2, getWindowHeight()/2, .0f), Vec3f(.0f, -1.0f, .0f));
	cam.lookAt(Vec3f(0, 0, 10), Vec3f(0, 0, 0));
	
	glPushMatrix();
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::enableAlphaBlending();
	gl::enableDepthRead(true);
	gl::enableDepthWrite(true);
	
	gl::setMatrices(cam);
	
	gl::translate(globalTranslate);
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		(*tile)->draw(pgen->particles);
	}
	
	
	
//	pgen->draw();
	
	//gl::draw(gl::Texture(surf_tank), getWindowBounds());
	glPopMatrix();
	
}

void hexApp::shutdown()
{
	delete tiles;
	delete dragging;
	delete pgen;
}

CINDER_APP_BASIC( hexApp, RendererGl )
