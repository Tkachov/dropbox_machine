#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "exit.h"

void show_error_message(string message, int error_code) {
	if(error_code == EXIT_NO_ERROR) {
		if(message != "") cerr << message << "\n";
	} else {
		if(message != "") cerr << message << ": " << strerror(error_code) << "\n";
		else cerr << strerror(error_code) << "\n";
	}
}

void show_error_message_and_exit(string message, int error_code) {
	show_error_message(message, error_code);
	exit(EXIT_FAILURE);
}
