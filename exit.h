#ifndef EXIT_H
#define EXIT_H

#include <errno.h>
#include <string>
using namespace std;

#define EXIT_NO_ERROR 0

void show_error_message(string message, int error_code);
void show_error_message_and_exit(string message, int error_code);

#endif

