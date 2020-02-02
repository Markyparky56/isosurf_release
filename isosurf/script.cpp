#include <iostream>
#include <map>
#include "script.h"
#include "parse.h"
#include "global.h"
#include "terrainwidget.h"

using namespace std;

struct Functor
{
	virtual void operator()(vector<string> args)
	{
	}
};
/*


struct teleportF : public Functor
{
	virtual void operator()(vector<string> args)
	{
		if (g.tactical && g.tactical->critters.size() && args.size() == 3)
			g.tactical->selected_unit()->pos(atof(args[0].c_str()), atof(args[1].c_str()), atof(args[2].c_str()));
	}
};

struct flyF : public Functor
{
	virtual void operator()(vector<string> args)
	{
		if (g.tactical && g.tactical->critters.size())
			g.tactical->selected_unit()->have_jetpack = !g.tactical->selected_unit()->have_jetpack;
	}
};




struct drawLinesF : public Functor
{
	virtual void operator()(vector<string> args)
	{
		g.draw_lines = !g.draw_lines;
	}
};
*/


struct quitF : public Functor
{
	virtual void operator()(vector<string> args)
	{
		g.running = false;
	}
};

map<string, Functor*> funcs;

void init_script()
{
	funcs["quit"] = new quitF;
}

void run_script(string input)
{
	vector<string> args;
	parseString(&args, input, " \t\n");
	removeEmptyStrings(&args);

	if (args.empty())
		return;

	string func_name = args[0];

	args.erase(args.begin());

	if (funcs.find(func_name) != funcs.end())
	{
		Functor *f = funcs[func_name];
		(*f)(args);
	}
}


void close_script()
{
	for (map<string, Functor*>::iterator it = funcs.begin(); it != funcs.end(); ++it)
		delete it->second;
	
	funcs.clear();
}

