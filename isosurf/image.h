#pragma once

#include <string>
#include "vect.h"

using namespace std;

struct Image
{
	Image(string fn);
	~Image();

	unsigned int width, height;
	unsigned int texWidth, texHeight;
	unsigned int gl_texid;
	string filename;

	void bind();
	void draw(vect2f &dst);
	void draw(vect2f &dst, vect2f &src, vect2f &dim);
};
