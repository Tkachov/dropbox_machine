#include "local_webserver.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
using std::cout;

#include <pthread.h>
#include <errno.h>

//I don't know anything about winsock2, so I just hope it does what sys/socket does

#include <fcntl.h>
//#include <sys/socket.h>
//#include <sys/types.h>
#include <winsock2.h>

#include <curl/curl.h>

#include "exit.h"
#include "machine.h"

local_webserver::local_webserver(int port, machine* ma): working(true), m(ma) {
	WSADATA wsaData;

    if (WSAStartup(MAKEWORD(1,1), &wsaData) == SOCKET_ERROR) {        
        show_error_message_and_exit("error initing WSA", 0);
    }

	listening_socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(listening_socket_desc == -1) show_error_message_and_exit("socket", errno);

	struct sockaddr_in server;
	server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(listening_socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
    	show_error_message_and_exit("bind", errno);

    listen(listening_socket_desc, 3);
};

local_webserver::~local_webserver() {
	close(listening_socket_desc);
}

extern "C" {
	void* process_client_request(void* args) {
		local_webserver* req = (local_webserver*)args;

		while(req->is_working()) {
			req->accept_new_requests();
		}
		
		pthread_exit(0);		
		return 0;
	}
}

void local_webserver::run() {
	pthread_t thread;
	if(pthread_create(&thread, NULL, process_client_request, this)) show_error_message_and_exit("pthread_create", errno);
}

void local_webserver::accept_new_requests() {	
	struct sockaddr_in client;
	int c = sizeof(struct sockaddr_in);

	int client_sock = accept(listening_socket_desc, (struct sockaddr*)&client, &c);
	if(client_sock < 0)	{
		if(errno == EINTR) {
			errno = 0;
			return;
		}

		show_error_message_and_exit("accept", errno);
	}

	work_with_client(client_sock);
}

#define BUFFER_SIZE 4096

void local_webserver::work_with_client(int fd) {
	//read HTTP request

	string contents = "";
	char buffer[BUFFER_SIZE];

	while(true) {		
		int read_size = recv(fd, buffer, BUFFER_SIZE, 0);
		if(read_size == 0) {					
			return; //I don't remember whether we must close(fd) here (it's already closed possibly)
		} 
		else if(read_size == -1) show_error_message_and_exit("recv", errno);
		else {
			buffer[read_size] = '\0';
			contents += buffer;
		}

		size_t left = contents.find("\r\n"), right = contents.rfind("\r\n");
		if(left != string::npos && right != string::npos && left != right) break;				
	}

	//parse ?code= out of it & request token
	//if everything's OK, answer "Success", otherwise send "Error"

	bool success = request_token(contents);
	string answer = "<html><head><title>Success</title></head><body><h1>Return to the machine now.</h1></body></html>";
	if(!success) answer = "<html><head><title>Error</title></head><body><h1>Some error happened, try again.</h1></body></html>";

	char buf[100] = {0};
	itoa(answer.size(), buf, 10);

	string output = "HTTP/1.0 200 OK\nContent-Length: ";
	output += buf;
	output += "\n\n";
	output += answer;

	while(true) {
		int write_size = send(fd, output.c_str(), output.size(), 0);
		if(write_size < 0) {
			if(errno == EPIPE) {
				errno = 0;
				cout << "EPIPE\n"; cout.flush();
				return; //EPIPE == client dropped
			} else show_error_message_and_exit("send", errno);
		} else {
			output = output.substr(write_size);			
			if(output.size() == 0) break;
		}
	}

	close(fd);

	//if everything's OK, we don't need this server thread anymore

	if(success) working = false;
}

string local_webserver::find_code(string contents) {
	string code = "";

	size_t index1 = contents.find("GET ");
	size_t index2 = contents.find(" HTTP/");	
	if(index1!=string::npos && index2!=string::npos) {
		string whole_url = contents.substr(index1+4, index2-index1-4);

		index1 = whole_url.find("code=");		
		if(index1!=string::npos) {
			index1 += 5;
			index2 = whole_url.find("&", index1);
			if(index2!=string::npos)
				code = whole_url.substr(index1, index2-index1);
			else
				code = whole_url.substr(index1);
		}
	}

	return code;
}

string data;

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) {
    for(int i=0; i<size*nmemb; ++i) data.push_back(buf[i]);
    return size*nmemb;
}

bool local_webserver::request_token(string contents) {
	const string APP_KEY = "dlm3nadaa4t3nsk"; //TODO load it
	const string APP_SECRET = "74wu3whu4zsom9t"; //TODO load it
	const string TOKEN_URL = "https://api.dropboxapi.com/1/oauth2/token";

	string code = find_code(contents);
	if(code == "") return false;

	CURL *curl;
	CURLcode res;	

	string url = TOKEN_URL;
	string post_fields = "code="+code;
	post_fields += "&grant_type=authorization_code";
	post_fields += "&client_id="+APP_KEY;
	post_fields += "&client_secret="+APP_SECRET;
	post_fields += "&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F";

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);

		data = "";
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return m->parse_json(data);
}