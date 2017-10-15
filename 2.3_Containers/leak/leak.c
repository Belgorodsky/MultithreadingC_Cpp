#include <stdlib.h>
#include <dmalloc.h>

void leak(unsigned int n) {
	(void)malloc(n);
}

int main (int argc, char **argv) {

	leak(1);
	leak(2);
	leak(3);

	void *m = malloc(1024);
	free(m);

	return 0;
}
