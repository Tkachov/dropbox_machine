#include "machine.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <pthread.h>
#include <unistd.h>

#include <curl/curl.h>

machine::machine(): working(true), server(0) {
	token = "";
	uid = "";

	cout << "+=================================================+\n";
	cout << "|                                                 |\n";
	cout << "| DDDD   RRRR    OOO   PPPP   BBBB    OOO   X   X |\n";
	cout << "| D   D  R   R  O   O  P   P  B   B  O   O   X X  |\n";
	cout << "| D   D  RRRR   O   O  PPPP   BBBB   O   O    X   |\n";
	cout << "| D   D  R  R   O   O  P      B   B  O   O   X X  |\n";
	cout << "| DDDD   R   R   OOO   P      BBBB    OOO   X   X |\n";
	cout << "|                                                 |\n";
	cout << "| M   M   AAA    CCCC  H   H  IIIII  N   N  EEEEE |\n";
	cout << "| MM MM  A   A  C      H   H    I    NN  N  E     |\n";
	cout << "| M M M  AAAAA  C      HHHHH    I    N N N  EEEEE |\n";
	cout << "| M   M  A   A  C      H   H    I    N  NN  E     |\n";
	cout << "| M   M  A   A   CCCC  H   H  IIIII  N   N  EEEEE |\n";
	cout << "|                                                 |\n";
	cout << "+=================================================+\n";
	cout << "\n";
};

machine::~machine() { delete server; }

void machine::work() {
	vector< pair<string, void(machine::*)()> > answers;
	if(token == "") answers.push_back(std::make_pair("Allow machine to use your Dropbox", turn_on));
	if(token != "") answers.push_back(std::make_pair("See Dropbox account info", info));
	answers.push_back(std::make_pair("Quit", quit));

	cout << "----\nSelect an action:\n";
	for(int i=0; i<answers.size(); ++i)
		cout << (i+1) << ": " << answers[i].first << "\n";
	cout << "----\n\n";
	cout.flush();

	int answer_index;
	cin >> answer_index;		
	if(answer_index > 0 && answer_index <= answers.size()) {
		(this->*answers[answer_index-1].second)();
	} else cout << "\n";
}

void machine::turn_on() {
	const string DROPBOX_URL = "https://www.dropbox.com/1/oauth2/authorize";
	const string APP_KEY = "dlm3nadaa4t3nsk"; //TODO load it

	if(token != "") {
		cout << "\nMachine already has access to your Dropbox.\n\n";
		cout.flush();
		return;
	}

	if(server == 0) {
		server = new local_webserver(12345, this);
		server->run();
		cout << "\n[The machine starts local webserver on port 12345]\n\n";
	} else cout << "\n[Local webserver is working on port 12345]\n\n";

	string url = DROPBOX_URL;
	url += "?response_type=code";
	url += "&redirect_uri=http://localhost:12345/"; //it's "http%3A%2F%2Flocalhost%3A12345%2F" if you want to open that url
	url += "&client_id=" + APP_KEY;

	cout << "Navigate to this URL and press \"Allow\":\n";
	cout << url << "\n\n";	
	cout.flush();
}

void machine::info() {
	cout << "\nThis is your account info:\n\n";
	cout.flush();

	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/1/account/info");

		string header = "Authorization: Bearer " + token;
		struct curl_slist *list = NULL;
  		list = curl_slist_append(list, header.c_str());
  		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);  

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_slist_free_all(list);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	cout << "\n\n";
	cout.flush();
}

string get_value(string json, string name) {
	name += "\": \"";

	size_t begin = json.find(name);
	if(begin == string::npos) return "";

	begin += name.size();
	size_t end = json.find("\"", begin);
	if(end == string::npos) return "";

	return json.substr(begin, end-begin);
}

bool machine::parse_json(string json) {
	token = get_value(json, "access_token");
	uid = get_value(json, "uid");

	if(token != "" && uid != "") {
		cout << "\nYou can see your Dropbox account info now.\nUse '0' to refresh actions list.\n\n";
		cout.flush();
		return true;
	}

	return false;
}