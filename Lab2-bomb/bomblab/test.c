#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

unsigned int _time=0;
unsigned int secs=0;

void sigint_handler(int sig)
{
	return;
}

unsigned int snooze()
{
	_time = sleep(secs);
	printf("Slept for %d of %d secs.\n", secs-_time, secs);
	return 0;
}

int main(int args, char *argv[])
{
	if(args==2){
		signal(SIGINT, sigint_handler);
		char *c_secs = argv[1];
		secs = c_secs[0]-48;
		printf("%d\n", secs);
		snooze();
	}
	return 0;
}
