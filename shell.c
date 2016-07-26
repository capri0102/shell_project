#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define	EOL		1
#define ARG		2
#define AMPERSAND	3
#define SEMICOLON	4

#define MAXARG		512
#define MAXBUF		512

#define FOREGROUND	0
#define BACKGROUND	1

static char special[] = {' ', '\t', '&', ';', '\n', '\0'};
static char inpbuf[MAXBUF], tokbuf[2*MAXBUF], *ptr = inpbuf, *tok = tokbuf;
char *prompt = "Command> ";

int userin(char *p);
int gettok(char **outptr);
int inarg(char c);
int procline(void);
int runcommand(char **cline, int where);


int main(void){
	while(userin(prompt) != EOF)
		procline();
}


int userin(char *p){
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;

	printf("%s", p);
	count = 0;

	while(1){
		if((c = getchar()) == EOF)
			return EOF;

		if(count < MAXBUF)
			inpbuf[count++] = c;
		if(c == '\n' && count < MAXBUF){
			inpbuf[count] = '\0';
			return count;
		}

		if(c == '\n'){
			printf("smallsh: input line too long\n");
			count = 0;
			printf("%s", p);
		}
	}
}

int gettok(char **outptr){
	int type;

	*outptr = tok;
	while(*ptr == ' ' || *ptr == '\t')
		ptr++;
	*tok++ = *ptr;

	switch(*ptr++){
		case '\n':
			type = EOL;
			break;
		case '&':
			type = AMPERSAND;
			break;
		case ';':
			type = SEMICOLON;
			break;
		default:
			type = ARG;
			while(inarg(*ptr))
				*tok++ = *ptr++;
	}

	*tok++ = '\0';
	return type;
}
int inarg(char c){
	char *wrk;
	for(wrk = special; *wrk; wrk++){
		if(c == *wrk)
			return 0;
	}

	return 1;
}
int procline(void){
	char *arg[MAXARG + 1];
	int toktype;
	int narg;
	int type;

	narg = 0;

	for(;;){
		switch(toktype = gettok(&arg[narg])){
			case ARG:	if(narg < MAXARG)
						narg++;
					break;
			case EOL:
			case SEMICOLON:
			case AMPERSAND:
				if(toktype == AMPERSAND)
					type = BACKGROUND;
				else
					type = FOREGROUND;

				if(narg != 0){
					arg[narg] = NULL;
					runcommand(arg, type);
				}

				if(toktype == EOL)
					return 0;
				narg = 0;
				break;
		}
	}
}

int runcommand(char **cline, int where){
	pid_t pid;
	int status;

	switch(pid = fork()){
		case -1:
			perror("smallsh");
			return -1;

		case 0:
			execvp(*cline, cline);
			perror(*cline);
			exit(1);
	}


	if(where == BACKGROUND){
		printf("[Process id %d]\n", pid);
		return 0;
	}

	if(waitpid(pid, &status, 0) == -1)
		return -1;
	else
		return status;
}
