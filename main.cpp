#include "machine.h"

int main() {
	machine m;
	while(m.is_working()) m.work();
	return 0;
}