#include "debugger.hpp"

Debugger* getActiveDebugger()
{
	static Debugger o;
	return &o;
}

Debugger::Debugger()
{
	this->fp.open("debugger.log", std::ios_base::out);
}
Debugger::~Debugger()
{
	this->fp.close();
}