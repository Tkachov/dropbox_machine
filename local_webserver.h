#ifndef LOCAL_WEBSERVER_H
#define LOCAL_WEBSERVER_H

#include <string>
using std::string;

class machine;

class local_webserver {
	bool working;
	int listening_socket_desc;
	machine* m;

	void work_with_client(int fd);
	bool request_token(string s);
	string find_code(string s);

public:
	local_webserver(int port, machine* m);
	~local_webserver();	
	void run();

	bool is_working() { return working; }
	void accept_new_requests();
};

#endif
