#include <stdio.h>

int main (int argc, char* argv[]) {
	int i;
	printf ("Entering %d grp = %d\n", getpid(), getpgrp());
	printf ("argc = %d\n", argc);
	for (i = 0; argv[i]; i++) {
		puts (argv[i]);
	}
}
