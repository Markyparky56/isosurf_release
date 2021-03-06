#include <iostream>
#include "global.h"
#include "script.h"
#include "terrainwidget.h"
#include "opengl.h"
#include "iso_common.h"
#include "iso_method_ours.h"
#include "mctable.h"
#include "csgtree.h"

#include "SDL.h"

#include <set>

int OVERSAMPLE_QEF = 4;
double BORDER = (1.0 / 16.0);
int DEPTH_MAX = 7; // 7
int DEPTH_MIN = 4; // 4
int FIND_ROOT_DEPTH = 0;


CSGNode *csg_root = 0;
using namespace std;

void writeFile(Mesh &m, string fn)
{
	FILE *f = fopen(fn.c_str(), "w");

#ifdef JOIN_VERTS
	map<TopoEdge, int> vt;

	int vert_num = 0;
	for (int i = 0; i < m.tris.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			map<TopoEdge, int>::iterator it = vt.find(m.topoTris[i][j]);
			if (it == vt.end())
			{
				vert_num++;

				vt[m.topoTris[i][j]] = vert_num;
				
				vect3f pos = m.tris[i][j];
				fprintf(f, "v %f %f %f\n", pos[0], pos[1], pos[2]);
			}
		}
	}

	set<vect<2, TopoEdge> > edges; // only for genus
	for (int i = 0; i < m.tris.size(); i++)
	{
		fprintf(f, "f %d %d %d\n", vt[m.topoTris[i][0]], vt[m.topoTris[i][1]], vt[m.topoTris[i][2]]);
		
		for (int j = 0; j < 3; j++)
		{
			vect<2, TopoEdge> e;
			e[0] = m.topoTris[i][j];
			e[1] = m.topoTris[i][(j+1) % 3];
			if (e[1] < e[0])
				swap(e[0], e[1]);
			edges.insert(e);
		}
	}

	// check genus
	int edge_num = edges.size();
	int face_num = m.tris.size();
	printf("verts = %d, edges = %d, faces = %d, genus = %d\n", vert_num, edge_num, face_num, vert_num - edge_num + face_num);
#else
	for (int j = 0; j < m.tris.size(); j++)
	{
		for (int i = 0; i < 3; i++)
			fprintf(f, "v %f %f %f\n", m.tris[j][i][0], m.tris[j][i][1], m.tris[j][i][2]);

		fprintf(f, "f %d %d %d\n", j*3+1, j*3+2, j*3+3);
	}
#endif
	fclose(f);
}

void init()
{
	init_script();

	// init SDL
	SDL_Init(SDL_INIT_VIDEO);
	g.screen = SDL_SetVideoMode(g.scr_width, g.scr_height, 32, SDL_HWSURFACE | SDL_OPENGL | SDL_HWACCEL
		| (g.fullscreen ? SDL_FULLSCREEN : 0));
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// some gl presets
	initOpenglFuncs();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// create widgets
	g.terrain_widget = new TerrainWidget();

	g.widget = g.terrain_widget;

	// define/load function
	if (!csg_root)
	{
		CSGNode *n;

		//n = new CSGMax(new CSGPlane(vect3f(.7,.7,.7 - .05), -~vect3f(1,1,1)),new CSGPlane(vect3f(.7,.7,.7), ~vect3f(1,1,1))); // used for dc topology figure depth 5 inverted
		//n = new CSGMin(new CSGPlane(vect3f(.7,.7,.7), -~vect3f(1,1,1)),new CSGPlane(vect3f(.7,.7,.7 - .05), ~vect3f(1,1,1))); // used for dc topology figure depth 5
		//n = new CSGSphere(vect3f(.5, .5, .5), .4);
		//n = new CSGTorus(vect3f(.5, .5, .5), .3, .1);
		//n = new CSGMin(new CSGSphere(vect3f(.5, .6, .35 - .01), .25),new CSGSphere(vect3f(.5, .5, .65 - .01), .25));
		//n = new CSGMax(new CSGSphere(vect3f(.5, .5, .3 - .05), .2),new CSGSphere(vect3f(.5, .5, .7 - .05), .2));
		//n = new CSGMin(new CSGSphere(vect3f(.5, .5, .65), .25),new CSGNeg(new CSGCylinder(vect3f(.5, .5, .35), vect3f(0,0,1), .15)));
		//n = new CSGMin(	new CSGSphere(vect3f(.5, .5, .5), .4),	new CSGPlane(vect3f(.65, .65, .55), vect3f(0,0,1))	);
		//n = new CSGPlane(vect3f(.65, .65, .65), vect3f(.2,.4,-1));

		// figure of interesting shapes in one cell
		//n = new CSGSphere(vect3f(.85, .25, .25), .1);
		//n = new CSGCylinder(vect3f(.85, .25, .25), vect3f(0,0,1), .1);
		//n = new CSGCylinder(vect3f(.85, .25, .25), ~vect3f(0.1,.7,.7), .1);
		//n = new CSGMax(new CSGPlane(vect3f(.75, .65, .15), -~vect3f(0,.5,1)),new CSGPlane(vect3f(.75, .65, .15), ~vect3f(0,0,1)));
		/*n = new CSGMax(
			new CSGPlane(vect3f(.75, .65, .15), -~vect3f(0,0,1)),
			new CSGPlane(vect3f(.75, .65, .15), ~vect3f(0.5,0,1))
			);*/
		/*n = new CSGMax(new CSGMax(
			new CSGPlane(vect3f(.35, .25, .35), -~vect3f(0,.5,1)),
			new CSGPlane(vect3f(.35, .25, .35), ~vect3f(0,0,1))),
			new CSGPlane(vect3f(.35, .25, .35), -~vect3f(.5,0,1)) );*/
		
		// draw box with things cut out (shifted) (mechanical part)
		vect3f shift(.062151346, .0725234, .0412);
		CSGNode *box = new CSGMin(new CSGMin(new CSGMin(new CSGPlane(vect3f(.3, .3, .3)+shift, vect3f(1,0,0)),
					 new CSGPlane(vect3f(.3, .3, .3)+shift, vect3f(0,1,0)) ),
				   new CSGMin(new CSGPlane(vect3f(.3, .3, .3)+shift, vect3f(0,0,1)),
					new CSGPlane(vect3f(.7, .7, .7)+shift, vect3f(-1,0,0)) )),
				   new CSGMin(new CSGPlane(vect3f(.7, .7, .7)+shift, vect3f(0,-1,0)),
					new CSGPlane(vect3f(.7, .7, .7)+shift, vect3f(0,0,-1))));
		n = new CSGMin(new CSGNeg(new CSGCylinder(vect3f(.5, .5, .5)+shift, vect3f(1,0,0), .15)),
					new CSGMin(new CSGNeg(new CSGCylinder(vect3f(.5, .5, .5)+shift, vect3f(0,1,0), .15)),
						new CSGMax(box, new CSGCylinder(vect3f(.5, .5, .5)+shift, vect3f(0,0,1), .15) ) )) ;
		n = new CSGMin(n, new CSGMin(new CSGPlane(vect3f(0, 0, .9)+shift, vect3f(0,0,-1)), new CSGPlane(vect3f(0, 0, .1)+shift, vect3f(0,0,1))));

		// $$$$$$ add it $$$$$$
		csg_root = n;
	}

	// generate surfaces
	initMCTable();

	gen_iso_ours(); // pregenerate tree
	gen_iso_ours();

	writeFile(g.ourMesh, "output.obj");
}

void run()
{
	g.widget->logic();
	g.time = SDL_GetTicks();

	while (g.running)
	{
		float sdl_time = SDL_GetTicks();
		while (g.time < sdl_time && g.running)
		{
			g.time += 10;

			g.widget->input();
			g.widget->logic();
		}

		g.widget->draw();

		SDL_GL_SwapBuffers();

		// simple framerate calculation
		float frame_time = SDL_GetTicks() - sdl_time;
		g.frame_time = g.frame_time * .9 + frame_time * .1;
	}
}

void halt()
{
	SDL_Quit();
	close_script();

	// save cursor pos
	FILE *f = fopen("cursor.txt", "w");
	fprintf(f, "%10.9f, %10.9f, %10.9f\n", g.cursor[0], g.cursor[1], g.cursor[2]);
	fclose(f);
}

int main(int argc, char **argv)
{
	::init();
	run();  
	halt();

	return 0;
}
