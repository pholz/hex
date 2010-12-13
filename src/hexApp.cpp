#include "ParticleGen.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include <iostream>
#include <fstream>
#include "boost/algorithm/string.hpp"
#include "cinder/Filesystem.h"
#include "OscListener.h"
#include "OscSender.h"
#include "cinder/Display.h"

#define OSC_SEND_HOST "localhost"
#define OSC_SEND_PORT 9000
#define OSC_RECEIVE_PORT 3000

using namespace ci;
using namespace ci::app;
using namespace std;

class hexApp : public AppBasic {

	vector< Tile* > *tiles;
	Tile* dragging;
	Tile* highlighted;
	Tile* lastSelected;
	Tile* navHighlighted;
	Vec2f dragoffset;
	vector<ParticleGen*> pgens;
	float last;
	Rand *rand;
	
	Surface surf_tank, surf_plane, surf_plane_inv, surf_dead;
	gl::Texture *tex_plane, *tex_plane_inv, *tex_dead;
	CameraOrtho cam;
	int maxid;
	float zoom;
	float defaultfactors[6];
	Vec3f globalTranslate;
	bool first;
	
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
	void oscSend(string address, float value);
	void highlightTile();
	void highlightTile(Tile* tile);
	void startAttack(int num);
	void moveHighlight(char dir);
};

void hexApp::prepareSettings(Settings* settings)
{
	//settings->setWindowSize(WIDTH, HEIGHT);
	//settings->setFullScreen(true);
	
	vector<shared_ptr<Display> > displays = Display::getDisplays();
	if(displays.size() > 1)
	{
		settings->setDisplay(displays[1]);
		settings->setFullScreen(true);
	}
	else
	{
		settings->setWindowSize(WIDTH, HEIGHT);
	}
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
	// init osc
	sender.setup(OSC_SEND_HOST, OSC_SEND_PORT);
	listener.setup(OSC_RECEIVE_PORT);
	
	oscSend("/cinder/osc/start", 1.0f);
	
	first = true;
	
	rand = new Rand();
	highlighted = NULL;
	dragging = NULL;
	lastSelected = NULL;
	navHighlighted = NULL;
	
	globalTranslate = Vec3f(.0f, .0f, .0f);
	
	cam = CameraOrtho(0, getWindowWidth(), getWindowHeight(), 0, 0, 1000);
	zoom = 1000.0f;
	
	// load textures
	surf_tank = Surface(loadImage(loadResource("tank.png")));
	surf_plane = Surface(loadImage(loadResource("plane.png")));
	surf_plane.setPremultiplied(false);
	surf_plane_inv = Surface(loadImage(loadResource("plane_inv.png")));
	surf_plane_inv.setPremultiplied(false);
	surf_dead = Surface(loadImage(loadResource("dead.png")));
	surf_dead.setPremultiplied(false);
	tex_plane = new gl::Texture(surf_plane);
	tex_plane_inv = new gl::Texture(surf_plane_inv);
	tex_dead = new gl::Texture(surf_dead);

	// init default hex vertex multipliers and states (latter unused)
	float df[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	memcpy(defaultfactors, df, 6 * sizeof(float));
	tiles = new vector< Tile* >();
	int s0[] = {1, 0, 0, 0, 0, 0};
	
	
	// read tile pos/phi/scale params from file
	// ---------------------------------------------------------------------------	
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
								  s0,
								  rand));
		
	}
	
	// ---------------------------------------------------------------------------

	
	
	last = getElapsedSeconds();
	navHighlighted = (*tiles)[4];
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
				if(highlighted)
				{
					highlighted->brightness = vol;
				}
								
			} catch (...) {
				console() << "Exception reading argument as float" << std::endl;
			}
		}
		else if(addr == "/max/doneplaying"  && message.getNumArgs() == 1)
		{
			try {
				
				
				float num = message.getArgAsFloat(0);
				console() << "GOT NUM " << num << endl;
				
				startAttack((int) num);
				
			} catch (...) {
				console() << "Exception reading argument as float" << std::endl;
			}
		}
	}
}

void hexApp::oscSend(string address, float value)
{
	osc::Message message;
	message.addFloatArg(value);
	message.setAddress(address);
	message.setRemoteEndpoint(OSC_SEND_HOST, OSC_SEND_PORT);
	sender.sendMessage(message);
}

void hexApp::highlightTile()
{
	Vec2f mpos = this->getMousePos();
	
	if(highlighted)
	{
		highlighted->highlighted = false;
		highlighted->brightness = .0f;
	}
		
	highlighted = NULL;
	
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		Vec2f collpos = mpos - (*tile)->pos;
		if(insidePolygon(collpos, (*(*tile)->hex), (*tile)->scale))
		{
			//console() << "dragging on" << endl;
			//dragoffset = collpos;
			highlighted = (*tile);
			highlighted->highlighted = true;
			
			oscSend("/cinder/osc/play", (float) highlighted->id);
		}
	}
}

void hexApp::highlightTile(Tile* tile)
{
	highlighted = tile;
	highlighted->highlighted = true;
	oscSend("/cinder/osc/play", (float) highlighted->id);
}

void hexApp::startAttack(int num)
{
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		Tile &t = *(*tile);
		
		if(t.id == num)
		{
			highlighted = NULL;
			t.highlighted = false;
			t.brightness = .7f;
			pgens.push_back(new ParticleGen(tiles, *tile, *tile, 90.0f, .7f, 20.0f, tex_plane, tex_plane_inv ));
		}
			
	}
}

void hexApp::keyDown( KeyEvent event )
{
	
	if(event.getChar() == ' ')
	{
		highlightTile();
	}
	
	if(navHighlighted && event.getChar() == '0')
	{
		
		if(highlighted == NULL)
			highlightTile(navHighlighted);
		else
			oscSend("/cinder/osc/no", 1.0f);
	}
	
	if(lastSelected && event.getChar() == 'a')
	{
		lastSelected->phi-=.5f;
	}
	else if(lastSelected && event.getChar() == 'd')
	{
		lastSelected->phi+=.5f;
	}
	else if(lastSelected && event.getChar() == 'w')
	{
		lastSelected->scale+=.01f;
	}
	else if(lastSelected && event.getChar() == 's')
	{
		lastSelected->scale-=.01f;
	}
	
	/*
	else if(event.getChar() == 'l')
	{
		globalTranslate.x += 1.0f;
	}
	else if(event.getChar() == '\'')
	{
		globalTranslate.x -= 1.0f;
	}
	else if(event.getChar() == '-')
	{
		globalTranslate.y += 1.0f;
	}
	else if(event.getChar() == ';')
	{
		globalTranslate.y -= 1.0f;
	}
	 */
	
	else if(lastSelected && event.getChar() == 'i')
	{
		lastSelected->ry-=.5f;
	}
	else if(lastSelected && event.getChar() == 'k')
	{
		lastSelected->ry+=.5f;
	}
	else if(lastSelected && event.getChar() == 'j')
	{
		lastSelected->rx-=.5f;
	}
	else if(lastSelected && event.getChar() == 'l')
	{
		lastSelected->rx+=.5f;
	}
	else if(lastSelected && event.getChar() == 'z')
	{
		lastSelected->z -= .05f;
	}
	else if(lastSelected && event.getChar() == 'x')
	{
		lastSelected->z += .05f;
	}
	else if(lastSelected && event.getChar() == '+')
	{
		lastSelected->pulseSpeed+=.1f;
	}
	else if(lastSelected && event.getChar() == '-')
	{
		lastSelected->pulseSpeed-=.1f;
	}
	else if(lastSelected && event.getChar() == 'c')
	{
		lastSelected->selectedCorner++;
		if(lastSelected->selectedCorner == 6) lastSelected->selectedCorner = -1;
	}
	else if(lastSelected && event.getChar() == '.' && lastSelected->selectedCorner >= 0)
	{
		lastSelected->hex->getPoints()[lastSelected->selectedCorner] += Vec2f( lastSelected->hex->getPoints()[lastSelected->selectedCorner].normalized() );
	}
	
	else if(lastSelected && event.getChar() == ',' && lastSelected->selectedCorner >= 0)
	{
		lastSelected->hex->getPoints()[lastSelected->selectedCorner] -= Vec2f( lastSelected->hex->getPoints()[lastSelected->selectedCorner].normalized() );
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
								  s0,
								  rand));
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
	else if(event.getChar() == 'l')
		moveHighlight('N');
	else if(event.getChar() == ';')
		moveHighlight('E');
	else if(event.getChar() == '\'')
		moveHighlight('S');
	else if(event.getChar() == '\\')
		moveHighlight('W');
	 
}

void hexApp::moveHighlight(char dir)
{
	
	if(!navHighlighted)
		return;
	
	Tile &hi = *navHighlighted;
	
//	vector<Tile*> g
	
	float min_delta = 100000.0f;
	Tile* min_tile = NULL;
	
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
	
		Tile &t = *(*tile);
		if(hi.id == t.id) continue; 
		
		
		
		switch(dir)
		{
			case 'E':
				if(t.pos.y > hi.pos.y - 100.0f && t.pos.y < hi.pos.y + 100.0f)
				{
					if(t.pos.x > hi.pos.x && (t.pos.x-hi.pos.x) < min_delta)
					{
						min_delta = t.pos.x-hi.pos.x;
						min_tile = *tile;
					}
				}
				break;
			case 'W':
				if(t.pos.y > hi.pos.y - 100.0f && t.pos.y < hi.pos.y + 100.0f)
				{
					if(t.pos.x < hi.pos.x && hi.pos.x-t.pos.x < min_delta)
					{
						min_delta = hi.pos.x-t.pos.x;
						min_tile = *tile;
					}
				}
				break;
			case 'S':
				if(t.pos.x > hi.pos.x - 100.0f && t.pos.x < hi.pos.x + 100.0f)
				{
					if(t.pos.y > hi.pos.y && t.pos.y-hi.pos.y < min_delta)
					{
						min_delta = t.pos.y-hi.pos.y;
						min_tile = *tile;
					}
				}
				break;
			case 'N':
				if(t.pos.x > hi.pos.x - 100.0f && t.pos.x < hi.pos.x + 100.0f)
				{
					if(t.pos.y < hi.pos.y && hi.pos.y-t.pos.y < min_delta)
					{
						min_delta = hi.pos.y-t.pos.y;
						min_tile = *tile;
					}
				}
				break;
			default:
				break;
		}

	}
	
	if(min_tile)
	{
		navHighlighted = min_tile;
	}
}

void hexApp::mouseDown( MouseEvent event )
{
	Vec2f mpos = Vec2f(event.getX(), event.getY());
	bool hit = false;
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		Vec2f collpos = mpos - (*tile)->pos;
		if(insidePolygon(collpos, (*(*tile)->hex), (*tile)->scale))
		{
			//console() << "dragging on" << endl;
			dragoffset = collpos;
			dragging = (*tile);
			lastSelected = (*tile);
			hit = true;
		}
	}
	
	if(!hit)
		lastSelected = NULL;
}

void hexApp::mouseUp( MouseEvent event )
{
	dragging = NULL;
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
	
	// get new messages
	oscUpdate();
	
	
	// update tiles
	// ---------------------------------------------------------------------------

	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		
		if(lastSelected == (*tile))
			(*tile)->selected = true;
		else
			(*tile)->selected = false;
		
		if(navHighlighted == (*tile))
			(*tile)->navHighlighted = true;
		else
			(*tile)->navHighlighted = false;
		
		if(highlighted == (*tile))
			(*tile)->highlighted = true;
		else
			(*tile)->highlighted = false;
		
		(*tile)->update(dt);
		 
	}
	// ---------------------------------------------------------------------------


	// update pgens
	// ---------------------------------------------------------------------------

	vector<ParticleGen*>::iterator pgen;
	for(pgen = pgens.begin(); pgen < pgens.end(); pgen++)
	{
		ParticleGen &pg = *(*pgen);
		
		// regular update
		pg.update(dt);
		
		// collide with all other particles 
		// ---------------------------------------------------------------------------

		vector<Particle*>::iterator partit;
		for(partit = pg.particles.begin(); partit < pg.particles.end(); partit++)
		{
			Particle &part = *(*partit);
			
			vector<ParticleGen*>::iterator pgen2;
			for(pgen2 = pgens.begin(); pgen2 < pgens.end(); pgen2++)
			{
				ParticleGen &pg2 = *(*pgen2);
				
				// only from other generators
				if(*pgen != *pgen2)
				{
					vector<Particle*>::iterator partit2;
					for(partit2 = pg2.particles.begin(); partit2 < pg2.particles.end(); partit2++)
					{
						Particle &part2 = *(*partit2);
						
						if(part.origin != part2.origin && part.pos.distance(part2.pos) < 20.0f && part.state != DYING && part2.state != DYING)
						{
							if(part.rand->nextInt(100) > part2.rand->nextInt(100))
							{
								part2.setState(DYING, true);
							}
							else
							{
								part.setState(DYING, true);
							}
							
							oscSend("/cinder/osc/fight", 1.0f);
						}
					}
				}
			}
		}
		// ---------------------------------------------------------------------------

		
		
		if(pg.ownExpired > pg.ownLifetime)
		{
			pgens.erase(pgen);
		}
	}
	
	// ---------------------------------------------------------------------------
}

void hexApp::draw()
{
	//cam.lookAt(Vec3f(getWindowWidth()/2, getWindowHeight()/2, -zoom), Vec3f(getWindowWidth()/2, getWindowHeight()/2, .0f), Vec3f(.0f, -1.0f, .0f));
	
	// ortho cam
	cam.lookAt(Vec3f(0, 0, 10), Vec3f(0, 0, 0));
	
	glPushMatrix();
	gl::clear( Color( 0, 0.0f, 0.0f ) ); 
	gl::enableAlphaBlending();
	gl::enableDepthRead(true);
	gl::enableDepthWrite(true);
	
	gl::setMatrices(cam);
	
	gl::translate(globalTranslate);
	
	
	// draw tiles
	vector<Tile*>::iterator tile;
	for(tile = tiles->begin(); tile < tiles->end(); tile++)
	{
		(*tile)->draw();
	}
	
	vector<Particle*>::iterator rem;
	for(rem = Particle::remains->begin(); rem < Particle::remains->end(); rem++)
	{
		(*rem)->setState(DEAD, true);
		
		if((*rem)->texture_dead == NULL)
			(*rem)->texture_dead = tex_dead;
		(*rem)->draw(.3f);
	}
	
	
	// draw pgens
	vector<ParticleGen*>::iterator pgen;
	for(pgen = pgens.begin(); pgen < pgens.end(); pgen++)
	{
		(*pgen)->draw();
	}
	
	
	glPopMatrix();
	
}

void hexApp::shutdown()
{
	oscSend("/cinder/osc/stop", 1.0f);
	delete tiles;
	delete dragging;
	pgens.clear();
}

CINDER_APP_BASIC( hexApp, RendererGl )
