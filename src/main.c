/*
 * @project Soundwave's simple shell
 * @file main.cpp
 * @author SoundWave
 * @date January 6, 2020
 * @brief Main file of project with main loop
 * */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <getopt.h>
#include <config.h>


//Colors:
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
//!Colors



/*
 * Special SWSH commands:
 * */
char *swsh_commands[] = {
	"cd",
	"help",
	"exit",
	"pwd"
};
//declarations of swsh commands:
int swsh_cd(char**);
int swsh_help(char**);
int swsh_exit(char**);
int swsh_pwd(char**);
int (*swsh_func[]) (char**) = {
	&swsh_cd, 
	&swsh_help,
	&swsh_exit,
	&swsh_pwd
};

int swsh_num_commands(){return sizeof(swsh_commands) /sizeof(char*); }

/* Functions implementations:*/
int swsh_cd(char** path){
	if(path[1] == NULL)
		fprintf(stderr, "swsh: expected argument to \"cd\"\n");
	else{
		if(chdir(path[1]) != 0)
			perror("swsh");
	}
	return 1;
}

int swsh_help(char **args){
	printf("SoundWave's shell\n");
	printf(".......\n");
	return 1;
}

int swsh_exit(char **args){return 0;}

char *get_pwd(void){
	char *return_string = get_current_dir_name();
	return return_string;
}

int swsh_pwd(char ** args){
	char *return_string = get_current_dir_name();
	printf("%s\n", return_string);
	return 1;
}
#define DEFAULT_HISTORY_SIZE 1024
char *greeting_line = "Welcome to SoundWave's shell\n";
char **history;
int history_size = DEFAULT_HISTORY_SIZE;
int history_iterator = 0;
void history_init(void){
	history = malloc(history_size * sizeof(char*));
	if(!history){
		fprintf(stderr, "swsh: allocating memory error");
		exit(EXIT_FAILURE);
	}
}
void history_add_command(char*command){
	if(history_iterator>= history_size){
		history_size += DEFAULT_HISTORY_SIZE;
		history = realloc(history, history_size);
	}

	history[history_iterator] = malloc((strlen(command)+1) * sizeof(char));
	strcpy(history[history_iterator], command);
	history_iterator++;
}
char* get_command(int n){
	if(n>history_iterator){
		return NULL;
	}
	return history[history_iterator-n];
	
}

//read_line:
#define SWSH_RL_BUFSIZE 1024 
char *read_line(void){
	int i =0;
	int current_buffer_size = SWSH_RL_BUFSIZE;
	char *buffer = malloc(sizeof(char) * current_buffer_size);
	int c;
	if(!buffer){
		fprintf(stderr, "swsh: allocating memory error\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		c = getchar();
		if(c == EOF || c=='\n'){
			buffer[i] = '\0';
			return buffer;
		}
		else{
			buffer[i] = c;
		}
		
		//check memory of buffer:
		if(i>=current_buffer_size){
			//reallocating:
			current_buffer_size += SWSH_RL_BUFSIZE;
			buffer = realloc(buffer, current_buffer_size);
			if(!buffer){
				fprintf(stderr, "swsh: allocating memory error\n");
				exit(EXIT_FAILURE);
			}
		}
		i++;

	}
}

//split line:

#define SWSH_TOKENS_BUFSIZE 64
#define DELIM " \t\r\n\a"
char **split_line(char *line){
	int current_token_size = SWSH_TOKENS_BUFSIZE;
	int i = 0;
	char **tokens = malloc(current_token_size * sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr, "swsh: allocating memory error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, DELIM);
	while(token!=NULL){
		tokens[i] = token;
		i++;
		if(i>= current_token_size){
			current_token_size += SWSH_TOKENS_BUFSIZE;
			tokens = realloc(tokens, current_token_size * sizeof(char*));
			if(!tokens){
				fprintf(stderr, "swsh: allocating memory error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, DELIM);
	}
	tokens[i] = NULL;
	return tokens;

}

//execute:
int execute_process(char ** args){
	pid_t pid, wpid;
	int return_status;
	pid = fork();
	if(pid == 0){
		//child process
		int exec_status = execvp(args[0]/*command*/, args/*agrs*/);
		if(exec_status == -1){
			perror("swsh");
			fprintf(stderr, "swsh: command not found\n");
		}
		
	}
	else if(pid<0){
		//error to create child process
		perror("swsh");
	}
	else{
		//Parent process:
		do{
			wpid = waitpid(pid, &return_status, WUNTRACED);
		}while(!WIFEXITED(return_status) && !WIFSIGNALED(return_status));
	}
	return 1;
}

//execute:
int execute(char **line){
	if(line[0] == NULL){
		return 1;
	}
	if(strcmp(line[0], "last") == 0){
		char* last_command = get_command(1);
		printf("%s\n", last_command);
		return 1;
	}
	for(int i = 0;i< swsh_num_commands();++i){
		if(strcmp(line[0], swsh_commands[i]) == 0){
			history_add_command(line[0]);
			return (*swsh_func[i])(line);
		}
	}
	return execute_process(line);
}



//main loop:
void swsh_loop(void){
	printf("%s",greeting_line);
	history_init();
	char *line; //pointer to input line;
	char **argline; // splited input line;
	int status;// return status of execute command;
	char hostname[255];
	do{
		gethostname(hostname, 255);
		uid_t uid;
		uid = getuid();
		struct passwd *pw;
		pw = getpwuid(uid);
		printf(RED"["BLU"%s@%s"RED"]"RESET,pw->pw_name ,hostname);
		printf(RED"%s"RESET,get_pwd());
		printf(">"); //greeting line
		line = read_line();
		argline = split_line(line);
		status = execute(argline);
		
		free(line); //free memory 
		free(argline);
	}while(status);
}



//entry point:
int main(){
	swsh_loop();
	return 0;
}


