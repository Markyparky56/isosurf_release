#include "image.h"
#include "assert.h"
#include "opengl.h"
#include <SDL.h>
#include <SDL_image.h>

Image::Image(string fn)
{
	filename = fn;

	// load image
	SDL_Surface *loadedSurface = IMG_Load(filename.c_str());
	if (!loadedSurface)
	{
		width = height = 0;
		gl_texid = 0;
		printf("failed to load texture: %s\n", fn.c_str());
		return;
	}

	width = loadedSurface->w;
	height = loadedSurface->h;

	// find size of texture
	texWidth = 1;
	texHeight = 1;

	while (texWidth < width)
		texWidth *= 2;

	while (texHeight < height)
		texHeight *= 2;

	// convert format
	int rmask, gmask, bmask, amask;
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;

	SDL_Surface *colorSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32, rmask, gmask, bmask, amask);
	SDL_Surface *surface = SDL_ConvertSurface(loadedSurface, colorSurface->format, SDL_SWSURFACE);

	// flip the surface vertically
	int *buf = new int [width];
	for (unsigned int i = 0; i < height/2; i++)
	{
		int *a = &((int*)surface->pixels)[i*(surface->pitch/4)];
		int *b = &((int*)surface->pixels)[(height - i - 1)*(surface->pitch/4)];
		memcpy(buf, a, width*4);
		memcpy(a, b, width*4);
		memcpy(b, buf, width*4);
	}
	delete buf;

	// store in expanded image
	int *pixels = new int [texWidth * texHeight];

	unsigned int y = 0;
	for (; y < height; y++)
	{
		unsigned int x = 0;
		for (; x < width; x++)
		{
			unsigned int val = ((int*)surface->pixels)[y*(surface->pitch/4) + x];
			if (val == 0xffff00ff)
				pixels[y*texWidth + x] = 0;
			else
				pixels[y*texWidth + x] = val;
		}

		for (; x < texWidth; x++)
			pixels[y*texWidth + x] = 0;
	}
	for (; y < texHeight; y++)
	{
		for (unsigned int x = 0; x < texWidth; x++)
			pixels[y*texWidth + x] = 0;
	}

	// hand to opengl
	glGenTextures(1, &gl_texid);
	glBindTexture(GL_TEXTURE_2D, gl_texid);

	//glTexImage2D(GL_TEXTURE_2D, 0, 4, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// free memory
	SDL_FreeSurface(colorSurface);
	SDL_FreeSurface(loadedSurface);
	SDL_FreeSurface(surface);
	delete pixels;
}

Image::~Image()
{
	glDeleteTextures(1, &gl_texid);
}

void Image::draw(vect2f &dst)
{
	// find texture coordinates
	float texX1 = 0;
	float texY1 = 0;
	float texX2 = width / (float)texWidth;
	float texY2 = height / (float)texHeight;

	// bind it
	glBindTexture(GL_TEXTURE_2D, gl_texid);

	// draw it
	glBegin(GL_QUADS);
	glTexCoord2f(texX1, texY1);
	glVertex2f(dst[0], dst[1]);

	glTexCoord2f(texX1, texY2);
	glVertex2f(dst[0], dst[1] + height);

	glTexCoord2f(texX2, texY2);
	glVertex2f(dst[0] + width, dst[1] + height);

	glTexCoord2f(texX2, texY1);
	glVertex2f(dst[0] + width, dst[1]);
	glEnd();
}

void Image::draw(vect2f &dst, vect2f &src_, vect2f &dim_)
{
	// fit into image
	vect2f src(max(0., min(src_[0], (float)width)), max(0., min(src_[1], (float)height)));
	vect2f dim(max(0., min(dim_[0], width - src_[0])), max(0., min(dim_[1], height - src_[1])));

	// find texture coordinates
	float texX1 = src[0] / (float)texWidth;
	float texY1 = src[1] / (float)texHeight;
	float texX2 = (src[0] + dim[0]) / (float)texWidth;
	float texY2 = (src[1] + dim[1]) / (float)texHeight;

	// bind it
	glBindTexture(GL_TEXTURE_2D, gl_texid);

	// draw it
	glBegin(GL_QUADS);
	glTexCoord2f(texX1, texY1);
	glVertex2f(dst[0], dst[1]);

	glTexCoord2f(texX1, texY2);
	glVertex2f(dst[0], dst[1] + dim[0]);

	glTexCoord2f(texX2, texY2);
	glVertex2f(dst[0] + dim[0], dst[1] + dim[0]);

	glTexCoord2f(texX2, texY1);
	glVertex2f(dst[0] + dim[0], dst[1]);
	glEnd();
}

void Image::bind()
{
	glBindTexture(GL_TEXTURE_2D, gl_texid);
}
