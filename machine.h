#ifndef MACHINE_H
#define MACHINE_H

#include "local_webserver.h"
#include <string>
using std::string;

class machine {	
	bool working;
	local_webserver* server;

	string token, uid;

	void info();
	void turn_on();
	void quit() { working = false; }

public:
	machine();
	~machine();	
	
	bool is_working() { return working; }
	void work();

	bool parse_json(string json);
};

#endif
