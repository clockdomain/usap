//
// reaper.c : Executes a list of programs  stored in a text file
// 	      and waits for its termination./
// Based on chapter 10 code in Linux programming by example.
//
//
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define UNUSED (-1) 
#define NKIDS (32)

typedef struct kid_st  {
	pid_t pid;
	char** argv;
} kid_t;

kid_t kids[NKIDS];
int nkids;
int nkidsleft;
static const char entry[] = "Entering handler\n";
static const char leave[] = "Leaving handler\n";


#define MAXCMD 80
char cmdLine[MAXCMD];

void childhandler (int sig);
int makeargv(const char *s, const char *delimiters, char ***argvp);

int main (int argc, char* argv[])
{
	struct sigaction sa;
	sigset_t childset, emptyset;
	int i;
	FILE* fp;
	char** myargv;
	char delim[] = " \t";

	if (argc < 2) {
		printf ("usage : %s <path-to-cmdfile>\n", argv[0]);
		exit(1);
	}
	fp = fopen (argv[1], "r");
	if (fp == NULL) {
		fprintf (stderr, "Cannot open : %s\n", argv[1]);
		exit(1);
	}

	for (i=0; i <NKIDS; i++) {	
		kids[i].pid = UNUSED;
	}
	sigemptyset (&emptyset);

	sa.sa_flags 	= SA_NOCLDSTOP;
	sa.sa_handler 	= childhandler;
	sigfillset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);

	sigemptyset (&childset);
	sigaddset (&childset, SIGCHLD);
	sigprocmask (SIG_SETMASK, &childset, NULL);

	int ret;

	while ((NULL != fgets (cmdLine, sizeof(cmdLine), fp)) && nkids < NKIDS) {
		cmdLine[strlen(cmdLine)-1] = 0;
		if ((ret = makeargv(cmdLine, delim, &kids[nkids].argv)) <= 0) {
			fprintf (stderr, "cant build argument list\n");
			exit(1);
		}
		if ((kids[nkids].pid = fork())== 0) {
			execvp (kids[nkids].argv[0], kids[nkids].argv);
			perror ("execvp");
			exit(1);
		}
		nkidsleft++;
		nkids++;
	}
	while (nkidsleft)
		sigsuspend (&emptyset);
	return 0;
}

#define NUMSZ 32
char buf[NUMSZ];
const char* format_num (int num)
{
int i;
	i = NUMSZ - 1;
	buf[i--] = 0;

	while (num) {
		buf[i--] = (num % 10) + '0';
		num = num / 10;
	}
	return &buf[i+1];
}

void childhandler (int sig) {
	int status, ret;
	int i;
	char buf[100];


	write (1, entry, strlen(entry));
	for (i = 0; i < nkids; i++) {
		if (kids[i].pid == UNUSED)
			continue;
		if ((ret = waitpid (kids[i].pid, &status, WNOHANG)) == kids[i].pid) {
			strcpy(buf, "\treaped process ");
			strcat(buf, format_num(ret));
			strcat(buf, "\n");
			write(1, buf, strlen(buf));
			kids[i].pid = UNUSED;
			nkidsleft--;
		}
	}
	write (1, leave, strlen(leave));
}
