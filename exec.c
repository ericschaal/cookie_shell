#include "headers.h"

/*
 * Role : Managing execution of commands.
 * start by reading cmd_dispatch() at the bottom.
 * 
 */

int run(char** cmd_tokens) {
	
	int process_table_id;
	process_table_id = available_process_index();
	
	if (process_table_id < 0) {
		printf("Process table full. Wait for a process to exit or kill one with command kill [pid].\n");
		return -1;
	}
	
	pid_t pid;
	pid = fork();
	
	if (pid < 0) {
		printf("Fork error. Child not created.\n");
		return -1;
	}
	
	else if (pid == 0) { // Child block
	
		setpgid(0, 0);
		
		int exc;
		if ((exc = execvp(*cmd_tokens, cmd_tokens)) < 0) {
			printf("Failed to execute command. Use help to display available commands.\n");
			exit(EXIT_FAILURE);
		} 
		exit(EXIT_SUCCESS);
	}
	// Main process
	if (bg_exec == 0) {
		
		tcsetpgrp(STDIN, pid); // allow stdin
		
		new_process(pid, *cmd_tokens, process_table_id);
		int status;
		waitpid(pid, &status, WUNTRACED);
		
		if (!WIFSTOPPED(status)) {
			if (remove_process(pid) < 0) {
				if (VERBOSE)
					printf("Process with pid %d not found\n", pid);
			}
		}
	
		tcsetpgrp(STDIN, my_pgid);
	}
	
	else {
		setpgid(pid, pid);
		printf("*[%d] \t PID[%d] \t %s started in the background\n", process_table_id, pid, *cmd_tokens);
		new_process(pid, *cmd_tokens, process_table_id);
		return 0;
	}
	return 0;
}


int run_pipe(char** parent_args, char** child_args) {
	
	if ((parent_args_c == 0) || (child_args_c == 0)) {
		printf("Syntax error: Usage [command] | [command2].\n");
		return -1;
	}
	
	int process_table_id_output, process_table_id_input;
	int fd[2];
	process_table_id_output = available_process_index();
	process_table_id_input = available_process_index();
	
	if ((process_table_id_input < 0) || (process_table_id_output < 0)) {
		printf("Process table full. Wait for a process to exit or kill one with command kill [pid].\n");
		free_proccess_index(process_table_id_output); // need to free to index we locked,
		return -1;
	}
	
	if (pipe(fd) < 0) {
		printf("Error while creating pipe\n");
		return -1;
	}
	
	pid_t pid_output, pid_input;

    if ((pid_output = fork()) < 0) {  // Fork to get a child.
        printf("Error while forking output process failed\n");
        return -1;
    }
	
	else if (pid_output == 0) {
		
		// as in class
		dup2(fd[STDOUT], STDOUT);
		close(fd[STDIN]);
		close(fd[STDOUT]);
		
		int exc;
		if ((exc = execvp(*parent_args, parent_args)) < 0) {
			printf("Failed to execute command %s. Use help to display available commands.\n", *parent_args);
			exit(EXIT_FAILURE);
		}
		
		exit(EXIT_SUCCESS);
	}
	
	if ((pid_input = fork()) < 0) {  // Fork to get a child.
        printf("Error while forking input process failed\n");
        return -1;
    }
	
	else if (pid_input == 0) {
		
		dup2(fd[STDIN], STDIN);
		close(fd[STDIN]);
		close(fd[STDOUT]);
		
		int exc;
		if ((exc = execvp(*child_args, child_args)) < 0) {
			printf("Failed to execute command %s. Use help to display available commands.\n", *child_args);
			exit(EXIT_FAILURE);
		}
		
		exit(EXIT_SUCCESS);
	}
	
	new_process(pid_output, *parent_args, process_table_id_output);
	new_process(pid_input, *child_args, process_table_id_input);
	
	int status_input, status_output;
	
	// to get EOF
	close(fd[STDIN]);
	close(fd[STDOUT]);
	
	waitpid(pid_output, &status_output, WUNTRACED);
	
	if (!WIFSTOPPED(status_output)) {
			if (remove_process(pid_output) < 0) {
				if (VERBOSE)
					printf("Process with pid %d not found. Exec might hava failed.\n", pid_output);
			}
	}
	
	waitpid(pid_input, &status_input, WUNTRACED);

	if (!WIFSTOPPED(status_input)) {
			if (remove_process(pid_input) < 0) {
				if (VERBOSE)
					printf("Process with pid %d not found. Exec might hava failed.\n", pid_input);
			}
	}

	
}



int run_redirect(char** full_cmd_tokens, int tk_count) {
	
	int process_table_id;
	process_table_id = available_process_index();
	
	if (process_table_id < 0) {
		printf("Process table full. Wait for a process to exit or kill one with command kill [pid].\n");
		return -1;
	}
	
	pid_t pid;
	char **cmd_tokens;

    if ((pid = fork()) < 0) {  // Fork to get a child.
        printf("Error while forking process failed\n");
        return -1;
    }
	
	else if (pid == 0) {
		
		if (tk_count <= 2) {
			printf("Invalid redirect: Usage [program] > [file].\n");
			return -1;
		}
		
		
		setpgid(0, 0);
		FILE *fp;
		
		fp = fopen(full_cmd_tokens[tk_count-1], "w+");
		dup2(fileno(fp), STDOUT_FILENO);
		fclose(fp);
	
		int x = 0;
		for (int i = 0; strcmp(full_cmd_tokens[i], ">\0") !=0 ; i++) {
			x++;
		}
		
		cmd_tokens = malloc(sizeof(char*)*(x+1));
		
		for (int i = 0; i < x; i++) { // getting command without >
			cmd_tokens[i] = full_cmd_tokens[i];
		}
		
		cmd_tokens[x]= '\0';
		execvp(*cmd_tokens, cmd_tokens);
		printf("Failed to execute command. Use help to display available commands.\n");
		exit(EXIT_FAILURE);
	}
	else {
		
		if (bg_exec == 0) {
			
			tcsetpgrp(STDIN, pid);
			
			new_process(pid, *full_cmd_tokens, process_table_id);
			int status;
			waitpid(pid, &status, WUNTRACED);
		
			if (!WIFSTOPPED(status)) {
				if (remove_process(pid) < 0)
					printf("Process with pid %d not found\n", pid);
			}
		
			tcsetpgrp(STDIN, my_pgid);
		}
		
		else {
			printf("*[%d] \t PID[%d] \t %s started in the background\n", process_table_id, pid, *full_cmd_tokens);
			new_process(pid, *full_cmd_tokens, process_table_id);
			return 0;
		}
		
		
	}
	
	return 0;
}



// redirect command to internal or external, checking params, ...
void cmd_dispatch(char **cmd_tokens, int tk_count, int pipe, int redirect) {
	
	if (tk_count > 0) {
		
		add_history(cmd_tokens, ln_count, tk_count, pipe, redirect);
		history_cleanup();
		ln_count++;
		
		if (strcmp(*cmd_tokens, "history\0") == 0) history_internal();
		else if (strcmp(*cmd_tokens, "jobs\0") == 0) jobs_internal();
		else if (strcmp(*cmd_tokens, "help\0") == 0) help_internal();
		else if (strcmp(*cmd_tokens, "kill\0") == 0) kill_internal(cmd_tokens, tk_count);
		else if (strcmp(*cmd_tokens, "fg\0") == 0) fg_internal(cmd_tokens, tk_count);
		else if (*cmd_tokens[0] == '!') enph_internal(cmd_tokens, tk_count);
		else if (strcmp(*cmd_tokens, "cd\0") == 0) cd_internal(cmd_tokens, pwd, base_dir);
		else if (strcmp(*cmd_tokens, "exit\0") == 0) exit_internal();
		else if ((strcmp(cmd_tokens[tk_count-1], "&\0") == 0))  {
			if  (tk_count <= 1) 
				printf("Invalid backgroud execution. Usage [program] &.\n");
			else {
				bg_exec = 1;
				free(cmd_tokens[tk_count-1]);
				cmd_tokens[tk_count-1] = NULL;
				tk_count--;
				if (redirect == 1) {
					run_redirect(cmd_tokens, tk_count);
				}
				else if (pipe == 1) // sorry about that
					printf("Piping in backgroud is unsupported.\n");
				else 	
					run(cmd_tokens); 
			}
		}
		else if (isalpha(**cmd_tokens)) { 
			if (redirect == 1) {
				run_redirect(cmd_tokens, tk_count);
			}
			else if (pipe == 1) {
					parsePipe(cmd_tokens, tk_count);
					run_pipe(parent_args, child_args);
					pipe_token_cleanup();
			}
			else {
				run(cmd_tokens);
			}
		}
	}
}


