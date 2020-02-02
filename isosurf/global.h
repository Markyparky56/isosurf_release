#pragma once

#include <string>
#include <vector>
#include <math.h>
#include "iso_common.h"
#include "player.h"
#include "vect.h"
#include "visitorextract.h"
using namespace std;


struct SDL_Surface;
struct TerrainWidget;
struct TNode;
struct Widget;


struct Mesh
{
	vector< vect<3, vect3f> > tris;
#ifdef JOIN_VERTS
	vector< vect<3, TopoEdge> > topoTris;
#endif
	vector<vect3f> norms;
};

struct Global
{
	Global();

	SDL_Surface* screen;

	TerrainWidget *terrain_widget;
	Widget *widget;

	TNode *ourRoot;

	Player player, camera;

	float time;
	float frame_time;
	float frame_rate() {return 1000.0 / frame_time;}
	int tris_drawn;

	bool draw_lines, draw_surf;
	bool draw_debug;
	bool camera_locked;
	vect3f cursor;

	int method;
	Mesh ourMesh, rootsMesh, dmcMesh, dcMesh;

	int scr_width, scr_height;
	bool fullscreen;
    bool running; // set false to quit the game
};

extern Global g;
