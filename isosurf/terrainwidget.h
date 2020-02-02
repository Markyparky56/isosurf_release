#pragma once

#include <vector>
#include "widget.h"

using namespace std;

struct TerrainWidget : public Widget
{
	virtual void input();
	virtual void logic();
	virtual void draw();
};
