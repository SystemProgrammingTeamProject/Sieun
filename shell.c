#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <setjmp.h>

#define BUFSIZE 256

sigjmp_buf jbuf;
pid_t pid;

void SIGQUIT_Handler(int signo){
	printf("\n");
	exit(1);
}

void SIGINT_Handler(int signo, pid_t pid){
	if(kill(pid, SIGTERM) != 0){
		printf("\n");
	}
}

int main(){
	signal(SIGINT, SIGINT_Handler);
	signal(SIGTSTP, SIGQUIT_Handler);

	char buf[BUFSIZE];
	int argc;
	int i = 0;

	while(1){
		char *argv[50] = { '\0' };
		printf("shell > ");
		gets(buf); // 명령어 입력

		/* Arguments 입력 받기 */
		argc = getargs(buf, argv);
		handler(argc, argv);
	}
}

/*
 * 입력받은 명령어를 구분해서 처리하는 함수
 */
void handler(int argc, char **argv) {

    int i = 0;
    int isBackground = 0, isRedirection = 0, isPipe = 0;
    for(i = 0; i < argc; i++) {
        if( (!strcmp(argv[i], ">")) || (!strcmp(argv[i], "<"))) {
            isRedirection = 1;
            break;
        }
        else if (!strcmp(argv[i], "|")) {
            isPipe = 1;
            break;
        }
        else if (!strcmp(argv[i], "&")) {
            isBackground = 1;
            break;
        }
    }

    if(isBackground){
	    launch(argc, argv);
	    isBackground = 0;
    } else if(!strcmp(argv[0], "ls")) {
				ls(argc, argv);
    } else if(!strcmp(argv[0], "cd")) {
				cd(argc, argv);
    } else if(!strcmp(argv[0], "pwd")){
				pwd();
    } else if(!strcmp(argv[0], "cat")){
    	cat(argc, argv);
    } else {
			launch(argc, argv);
    }
}

/*
 * list Function
 */
void ls(int argc, char **argv){
	char buf[256];
	if(argc == 1){
		getcwd(buf, 256);
		printf("%s", buf);
		argv[1] = buf;
	}

	DIR *pdir;
	if((pdir = opendir(argv[1])) < 0){
		perror("[ ERROR ] OPEN DIR: ");
	}

	printf("\n");

	int i = 0;
	struct dirent *pde;

	while((pde = readdir(pdir)) != NULL){
		printf("%-20s", pde->d_name);
		if(++i % 3 == 0) printf("\n");
	}

	printf("\n");

	closedir(pdir);
}

void pwd(){
	char *buf = (char *)malloc(sizeof(char) * BUFSIZE);

	if(getcwd(buf, BUFSIZE) == NULL){
		perror("[ ERROR ] pwd");
		exit(EXIT_FAILURE);
	} else
		printf("%s \n", buf);

	free(buf);
}

void cd(int arg_cnt, char **argv){
	if(arg_cnt == 1){
		chdir("HOME");
	} else {
		if(chdir(argv[1]) == -1){
			printf("%s : No Search File or Directory\n", argv[1]);
		}
	}
}

void cat(int argc, char **argv){
	if(argc < 2){
		fprintf(stderr, "Please Input Files : \n");
		exit(1);
	}

	FILE *file;
	for(int i = 1; i < argc; i++){
		file = fopen(argv[i], "r");
		if(file == NULL){
			printf("Cat: %s: No Such File or Directory\n", argv[i]);
			continue;
		}

		int buf;
		while((buf = fgetc(file)) != EOF) {
			putchar(buf);
		}

		printf("\n");
		if(fclose(file) != 0){
			perror("[ ERROR ] Closing File");
		}
	}
}

/*
 * 백그라운드에서 실행하도록 하는 함수
 */
void launch(int arg_cnt, char **argv){
	pid = 0;
	int i = 0;
	int isBackground = 0;

	if(arg_cnt != 0 && !strcmp(argv[arg_cnt - 1], "&")){
		argv[arg_cnt - 1] = NULL;
		isBackground = 1;
	}

	pid = fork();

	if(pid == 0){
		if(isBackground){
			printf("\n[ CREATE ] BACKGROUND PROCESS(%ld)\n", (long)getpid());
		}

		if(execvp(argv[0], argv) < 0){
			perror("[ ERROR ] CREATE BACKGROUND");
		}
	} else {
		if(isBackground == 0){
			wait(pid);
		}
	}
}

/*
 * Return the number of Arguments
 */
int getargs(char *cmd, char **argv)
{
    int argc = 0;
    while (*cmd)
    {
        if (*cmd == ' ' || *cmd == '\t')
            *cmd++ = '\0';
        else
        {
            argv[argc++] = cmd++;
            while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
                cmd++;
        }
    }
    argv[argc] = NULL;
    return argc;
}

