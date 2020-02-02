#include "terrainwidget.h"
#include "opengl.h"
#include "global.h"
#include "iso_method_ours.h"
#include "traverse.h"
#include "vect.h"
#include "matrix.h"
#include "index.h"
#include <SDL.h>
#include "timer.h"
#include "visitorextract.h"
#include <fstream>

void TerrainWidget::input()
{
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(0);

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		int steps = 32;
		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				g.running = false;
			}
			else if (event.key.keysym.sym == SDLK_TAB)
			{
				g.method = (g.method + 1) % 4;
			}
			else if (event.key.keysym.sym == SDLK_LEFT)
			{
				g.cursor[0] -= 1.0/steps;
			}
			else if (event.key.keysym.sym == SDLK_RIGHT)
			{
				g.cursor[0] += 1.0/steps;
			}
			else if (event.key.keysym.sym == SDLK_UP)
			{
				g.cursor[1] += 1.0/steps;
			}
			else if (event.key.keysym.sym == SDLK_DOWN)
			{
				g.cursor[1] -= 1.0/steps;
			}
			else if (event.key.keysym.sym == SDLK_PAGEUP)
			{
				g.cursor[2] += 1.0/steps;
			}
			else if (event.key.keysym.sym == SDLK_PAGEDOWN)
			{
				g.cursor[2] -= 1.0/steps;
			}

			else if (event.key.keysym.sym == SDLK_o)
			{
				//read camera
				ifstream f("camera.txt");
				f >> g.player.pos[0] >> g.player.pos[1] >> g.player.pos[2];
				f >> g.player.pitch >> g.player.yaw;
				f.close();
			}
			else if (event.key.keysym.sym == SDLK_p)
			{
				//write camera
				FILE *f = fopen("camera.txt", "w");
				fprintf(f, "%f %f %f\n", g.player.pos[0], g.player.pos[1], g.player.pos[2]);
				fprintf(f, "%f %f\n", g.player.pitch, g.player.yaw);
				fclose(f);
			}
			
			else if (event.key.keysym.sym == SDLK_l)
			{
				g.draw_lines = !g.draw_lines;
			}
			
			else if (event.key.keysym.sym == SDLK_k)
			{
				g.draw_surf = !g.draw_surf;
			}
			else if (event.key.keysym.sym == SDLK_g)
			{
				g.draw_debug = !g.draw_debug;
			}
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			float &pitch = g.player.pitch;
			float &yaw = g.player.yaw;

			pitch += event.motion.yrel / -180.0;
			yaw += event.motion.xrel / -180.0;

			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}
		else if(event.type == SDL_QUIT)
		{
			g.running = false;
		}
	}

	// read states
	Uint8 *keystate = SDL_GetKeyState(0);

	vect3f walk_dir(0,0,0);
	vect3f fwd = g.player.getForward();
	vect3f rt = g.player.getRight();

	float speed = .005;
	if (keystate[SDLK_LCTRL])
		speed *= 10;

	if (keystate['w'])
		walk_dir += fwd*speed;
	if (keystate['s'])
		walk_dir -= fwd*speed;
	if (keystate['d'])
		walk_dir += rt*speed;
	if (keystate['a'])
		walk_dir -= rt*speed;
	if (keystate[' '])
		walk_dir[2] += speed;
	if (keystate[SDLK_LSHIFT])
		walk_dir[2] -= speed;

	g.player.pos += walk_dir;
}

void TerrainWidget::logic()
{
}

struct VisitorDebug
{
	bool on_vert(TraversalData &a, TraversalData &b, TraversalData &c, TraversalData &d,
		TraversalData &aa, TraversalData &ba, TraversalData &ca, TraversalData &da){return false;}

	bool on_node(TraversalData &td)
	{
#if 0 // dont draw boxes vs draw boxes
		return !td.n->is_leaf();
#else
		if (td.n->is_leaf())
		{
			if (td.n->verts[0][0] >= 0 && td.n->verts[7][0] <= 1&&
				td.n->verts[0][1] >= 0 && td.n->verts[7][1] <= 1&&
				td.n->verts[0][2] >= 0 && td.n->verts[7][2] <= 1)
			{
			if (td.n->changesSign())
			{
				glDisable(GL_LIGHTING);

				// outside lines
				glLineWidth(2);
				glBegin(GL_LINES);
				for (Index i; i < 4; i++)
				{
					// cell edges
					glColor3f(.5,0,0);

					glVertex3fv(td.n->verts[Index(i.x, i.y, 0)].v);
					glVertex3fv(td.n->verts[Index(i.x, i.y, 1)].v);

					glVertex3fv(td.n->verts[Index(i.x, 0, i.y)].v);
					glVertex3fv(td.n->verts[Index(i.x, 1, i.y)].v);

					glVertex3fv(td.n->verts[Index(0, i.x, i.y)].v);
					glVertex3fv(td.n->verts[Index(1, i.x, i.y)].v);

					// gradients
					/*glColor3f(0,.5,0);
					for (int i = 0; i < 8; i++)
					{
					vect3f v = td.n->verts[i];
					glVertex3fv(v.v);
					v += gradient(v) * .05;
					glVertex3fv(v.v);
					}*/
				}
				glEnd();

				glColor3f(.7, .7, .7);
				glEnable(GL_LIGHTING);
			}
			}
			return false;
		}
		return true;
#endif
	}

	bool on_edge(TraversalData &td00, TraversalData &td10, TraversalData &td01, TraversalData &td11, char orient)
	{
		if (td00.n->is_leaf() && td10.n->is_leaf() && td01.n->is_leaf() && td11.n->is_leaf())
		{
			// determine which node is smallest (ordered bitwise)
			TNode *n[4] = {td00.n, td10.n, td01.n, td11.n};
			int depths[4] = {td00.depth, td10.depth, td01.depth, td11.depth};
			int small = 0;
			for (int i = 1; i < 4; i++)
			{
				if (depths[i] > depths[small])
					small = i;
			}

			// calc edge vertex
			int edge_idx = cube_orient2edge[orient][small ^ 3];
			vect4f &edgeMid = n[small]->edges[edge_idx];
			vect4f &edgeLow = n[small]->verts[cube_edge2vert[edge_idx][0]];
			vect4f &edgeHigh = n[small]->verts[cube_edge2vert[edge_idx][1]];

			// create pyramids in each cell
			for (int i = 0; i < 4; i++)
			{
				if (n[i]->is_outside())
					continue;

				static int faceTable[3][4][2] =
				{
					{
						{1,0},{4,0},{1,5},{4,5}
					},
					{
						{2,0},{3,0},{2,5},{3,5}
					},
					{
						{2,1},{3,1},{2,4},{3,4}
					}
				};

				static bool flipTable[3][4] =
				{
					{true,false,false,true},
					{false,true,true,false},
					{true,false,false,true}
				};

				// find min size face
				const int i1 = i ^ 1;
				const int i2 = i ^ 2;
				bool do1 = n[i] != n[i1];
				bool do2 = n[i] != n[i2];
				vect4f face1, face2;

				if (depths[i] == depths[i1])
					face1 = (n[i]->faces[faceTable[orient][i^3][0]] + n[i1]->faces[cube_face2opposite[faceTable[orient][i^3][0]]]) * .5;
				else if (depths[i] > depths[i1])
					face1 = n[i]->faces[faceTable[orient][i^3][0]];
				else
					face1 = n[i1]->faces[cube_face2opposite[faceTable[orient][i^3][0]]];

				if (depths[i] == depths[i2])
					face2 = (n[i]->faces[faceTable[orient][i^3][1]] + n[i2]->faces[cube_face2opposite[faceTable[orient][i^3][1]]]) * .5;
				else if (depths[i] > depths[i2])
					face2 = n[i]->faces[faceTable[orient][i^3][1]];
				else
					face2 = n[i2]->faces[cube_face2opposite[faceTable[orient][i^3][1]]];

#if 1
				if (n[i]->contains(g.cursor)) 
				//if (vect3f(.5, .5, .125) == vect3f(edgeMid))
				{
				glDisable(GL_LIGHTING);

#if 0 // draw samples with points vs spheres
				glPointSize(50);

				glBegin(GL_POINTS);
				glColor3f(1,0,1);
				glVertex3fv(n[i]->node.v);
				glColor3f(0,0,1);
				glVertex3fv(face1.v);
				glVertex3fv(face2.v);
				glColor3f(1,1,0);
				glVertex3fv(edgeMid.v);
				glColor3f(0,0,0);
				glVertex3fv(edgeLow.v);
				glVertex3fv(edgeHigh.v);
				glEnd();
#else
				glColor3f(1,0,1);
				drawSphere(n[i]->node);
				glColor3f(0,0,1);
				drawSphere(face1);
				drawSphere(face2);
				glColor3f(1,1,0);
				drawSphere(edgeMid);
				glColor3f(0,0,0);
				drawSphere(edgeLow);
				drawSphere(edgeHigh);
#endif

				glColor3f(0,0,0);
				glLineWidth(4);
				glBegin(GL_LINES);
#if 1 // draw edges of tets
				if (do1) 
				{
					// on face
					glVertex3fv(edgeHigh.v);
					glVertex3fv(face1.v);
					glVertex3fv(edgeLow.v);
					glVertex3fv(face1.v);
					glVertex3fv(edgeMid.v);
					glVertex3fv(face1.v);

					// off face
					glVertex3fv(n[i]->node.v);
					glVertex3fv(face1.v);
				}
				if (do2)
				{
					// on face
					glVertex3fv(edgeHigh.v);
					glVertex3fv(face2.v);
					glVertex3fv(edgeLow.v);
					glVertex3fv(face2.v);
					glVertex3fv(edgeMid.v);
					glVertex3fv(face2.v);

					// off face
					glVertex3fv(n[i]->node.v);
					glVertex3fv(face2.v);
				}

				// cell edge (tet)
				glVertex3fv(n[i]->node.v);
				glVertex3fv(edgeHigh.v);
				glVertex3fv(n[i]->node.v);
				glVertex3fv(edgeLow.v);
				glVertex3fv(n[i]->node.v);
				glVertex3fv(edgeMid.v);
#endif
				//glVertex3fv(n[i1]->node.v);
				//glVertex3fv(n[i2]->node.v);

				// cell edge
				glColor3f(.5, 0, 0);
				glVertex3fv(edgeLow.v);
				glVertex3fv(edgeHigh.v);

				glEnd();

				glEnable(GL_LIGHTING);
				glColor3f(.7, .7, .7);
				}
#endif
			}
			return false;
		}

		return true;
	}

	bool on_face(TraversalData &td0, TraversalData &td1, char orient)
	{
		return !(td0.n->is_leaf() && td1.n->is_leaf());
	}

	void drawSphere(vect3f center)
	{
		float size = .025;
		vect3f x(1,0,0), y(0,1,0), z(0,0,1), u, p;
		float a, b;
		const int n = 40, m = n/2;

		for (int i = 0; i < n; i++)
		{
			glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0; j < m; j++)
			{
				a = 2*pi*i/n;
				b = 2*pi*j/m;

				u = x*cos(a) + y*sin(a);
				p = z*cos(b) + u*sin(b);

				glVertex3fv((center+p*size).v);
				

				a = 2*pi*(i+1)/n;
				b = 2*pi*j/m;

				u = x*cos(a) + y*sin(a);
				p = z*cos(b) + u*sin(b);

				glVertex3fv((center+p*size).v);
			}
			glEnd();
		}
	}
};

void TerrainWidget::draw()
{
	// clear
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	// matrix stuff
	float aspect = float(g.scr_width) / g.scr_height;
	float fov_y = 40;
	float fov_x = atan(tan(fov_y*pi/180) * aspect) * 180/pi;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov_y, aspect, .001, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float light_pos[4] = {0, 0, 1, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	vect3f pos = g.player.pos;
	vect3f fwd = g.player.pos + g.player.getForward();
	vect3f up = g.player.getUp();
	gluLookAt(pos[0], pos[1], pos[2], fwd[0], fwd[1], fwd[2], up[0], up[1], up[2]);

	// draw
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glDisable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glDisable(GL_TEXTURE_2D);

	//if (g.terrain)
	{
		g.player.n1 = matrix3f::rotaxis(g.player.getRight(), -fov_x*.5*pi/180).mult(g.player.getUp());
		g.player.n2 = matrix3f::rotaxis(g.player.getRight(), fov_x*.5*pi/180).mult(-g.player.getUp());
		g.player.n3 = matrix3f::rotaxis(g.player.getUp(), fov_y*.5*pi/180).mult(g.player.getRight());
		g.player.n4 = matrix3f::rotaxis(g.player.getUp(), -fov_y*.5*pi/180).mult(-g.player.getRight());

		if (!g.camera_locked)
			g.camera = g.player;

		// draw triangles
		Mesh *m = &g.ourMesh;
		if (g.method == 1)
			m = &g.rootsMesh;
		else if (g.method == 2)
			m = &g.dmcMesh;
		else if (g.method == 3)
			m = &g.dcMesh;

		if (g.draw_surf)
		{
			glColor3f(.7,.7,.7);
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < m->tris.size(); i++)
			{
				glNormal3fv(m->norms[i].v);
				for (int j = 0; j < 3; j++)
					glVertex3fv(m->tris[i][j].v);
			}
			glEnd();
		}

		if (g.draw_lines)
		{
			glLineWidth(2);
			glDisable(GL_LIGHTING);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonOffset(-.1, -.1);

			glColor3f(0,0,.3);
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < m->tris.size(); i++)
			{
				for (int j = 0; j < 3; j++)
					glVertex3fv(m->tris[i][j].v);
			}
			glEnd();
			
			glDisable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// draw debug stuff
		//if (g.draw_debug)
		//{
		//	//glEnable(GL_BLEND);
		//	//glEnable(GL_POINT_SMOOTH);
		//	//glEnable(GL_LINE_SMOOTH);

		//	glLineWidth(2);
		//	VisitorDebug v;
		//	TraversalData td(g.ourRoot);
		//	traverse_node<trav_edge>(v, td);
		//}
	}

	glDisable(GL_LIGHTING);

	//// draw unit cube
	//glLineWidth(2);
	//glColor3f(0,0,.75);
	//glBegin(GL_LINES);
	//glColor3f(0,0,1);
	//glVertex3f(0,0,0);
	//glVertex3f(0,0,1);

	//glColor3f(0,1,0);
	//glVertex3f(0,0,0);
	//glVertex3f(0,1,0);

	//glColor3f(1,0,0);
	//glVertex3f(0,0,0);
	//glVertex3f(1,0,0);

	//glColor3f(0,0,.75);
	//glVertex3f(0,1,0);
	//glVertex3f(0,1,1);
	//glVertex3f(1,0,0);
	//glVertex3f(1,0,1);
	//glVertex3f(1,1,0);
	//glVertex3f(1,1,1);
	//glVertex3f(0,0,1);
	//glVertex3f(0,1,1);
	//glVertex3f(1,0,0);
	//glVertex3f(1,1,0);
	//glVertex3f(1,0,1);
	//glVertex3f(1,1,1);
	//glVertex3f(0,0,1);
	//glVertex3f(1,0,1);
	//glVertex3f(0,1,0);
	//glVertex3f(1,1,0);
	//glVertex3f(0,1,1);
	//glVertex3f(1,1,1);
	//glEnd();

	//// draw nullspace
	//vect3f ns_p(0.32993808,0.35556808,0.13031913);
	//vect3f ns_v(-0.38332221,-0.32238770,0.26032475);
	//vect3f ns_v2(-0.071428705870276249,0.84920804557651974,-0.0012291251482604748);
	//
	//glBegin(GL_LINES);
	//glColor3f(0,1,1);
	//glVertex3fv((ns_p-ns_v*10).v);
	//glVertex3fv((ns_p+ns_v*10).v);
	
	//glColor3f(0,1,.5);
	//glVertex3fv((ns_p-ns_v2).v);
	//glVertex3fv((ns_p+ns_v2).v);
	//glEnd();
	//
	//glBegin(GL_POINTS);
	//glColor3f(1,0,0);
	//glVertex3fv(ns_p.v);
	//glEnd();

	//// draw cursor
	//glPointSize(10);
	//glBegin(GL_POINTS);
	//glColor3f(1,0,0);
	//glVertex3fv(g.cursor.v);
	//glEnd();
	
	//// HUD
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(0, g.scr_width, 0, g.scr_height, 10, -10);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	//glDisable(GL_CULL_FACE);
	//glDisable(GL_LIGHTING);
	//glEnable(GL_TEXTURE_2D);

	//glColor3f(0,0,0);
	//char buf[1024];
	//sprintf(buf, "cursor %f, %f, %f\n", g.cursor[0], g.cursor[1], g.cursor[2]);
	//g.font_small->draw(buf, vect2f(0, 0));

	//char *methodnames[] = {"Ours", "Roots", "DMC", "DC"};
	//sprintf(buf, "method %s\n", methodnames[g.method]);
	//g.font_small->draw(buf, vect2f(0, 16));
	
}
