#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>


/*
 * Special SWSH commands:
 * */
char *swsh_commands[] = {
	"cd",
	"help",
	"exit"
};
//declarations of swsh commands:
int swsh_cd(char**);
int swsh_help(char**);
int swsh_exit(char**);
int (*swsh_func[]) (char**) = {
	&swsh_cd, 
	&swsh_help,
	&swsh_exit
};

int swsh_num_commands(){return sizeof(swsh_commands) /sizeof(char*); }

/* Functions implementations:*/
int swsh_cd(char** path){
	if(path[1] == NULL)
		fprintf(stderr, "swsh: expected argument to \"cd\"\n");
	else{
		if(chdir(path[1]) != 0)
			perror("lsh");
	}
	return 1;
}

int swsh_help(char **args){
	printf("SoundWave's shell\n");
	printf(".......");
	return 1;
}

int swsh_exit(char **args){return 0;}





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
	for(int i = 0;i< swsh_num_commands();++i){
		if(strcmp(line[0], swsh_commands[i]) == 0){
			return (*swsh_func[i])(line);
		}
	}
	return execute_process(line);
}



//main loop:
void swsh_loop(void){
	char *line; //pointer to input line;
	char **argline; // splited input line;
	int status;// return status of execute command;
	do{
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


