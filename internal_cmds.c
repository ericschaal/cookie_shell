#include "headers.h"

/*
 * All this commands are internal to cookie_shell 
 * 
 * 
 */

int cd_internal(char** cmd_tokens, char* pwd, char* base_dir) { // simple change directory (cd) implementation
	
	if (cmd_tokens[1] == NULL) {
		chdir(base_dir);
		strcpy(pwd, base_dir);
		pwd_rw();
		return 0;
	}
	else if (chdir(cmd_tokens[1]) == 0) {
		getcwd(pwd, MAX_BUFFER_SIZE);
		pwd_rw();
		return 0;
	}
	else if ((strcmp(cmd_tokens[1], "~\0") == 0) || (strcmp(cmd_tokens[1], "~/\0") == 0)) {
		chdir(base_dir);
		strcpy(pwd, base_dir);
		pwd_rw();
		return 0;
	}
	else {
		printf("Invalid location %s\n", cmd_tokens[1]);
	}
}

int kill_internal(char** cmd_tokens, int tk_count) { // kills a process identified by pid
	if (tk_count < 2) {
		printf("Syntax error, invalid use of kill. Usage : kill pid\n");
		return -1;
	}
	
	int pid = strtol(cmd_tokens[1], NULL, 10);
	
	if (pid <= 0) {
		printf("Invalid pid.\n");
		return -1;
	}
	
	process* selected = find_process(pid);

	if ((selected == NULL) || (selected->active == 0)) {
		printf("Couldn't find active process with pid %d\n", pid);
		return -1;
	}
	
	killpg(pid, SIGTERM);
	
	return 0;
}

int jobs_internal() { // prints jobs in background
	for (int i = 0; i < MAX_JOBS; i++) {
		if (process_table[i].active == 1)
			printf("[%d] \t PID[%d] \t %s.\n", i, process_table[i].pid, process_table[i].name);
	}
	return 0;
}

int fg_internal(char **cmd_tokens, int tk_count) { // brings a background job to the foreground.
	
	if (tk_count < 2) {
		printf("Syntax error, invalid use of fg. Usage : fg pid\n");
		return -1;
	}
	
	int job_pid = strtol(cmd_tokens[1], NULL, 10); // get the pid (as an int).
	
	if (job_pid <= 0) { //invalid pid : NaN, negative ...
		printf("Invalid pid.\n");
		return -1;
	}
	
	process* selected = find_process(job_pid); // find the process identified by pid.
	
	if (selected == NULL) {
		printf("Cannot find proccess with pid %d.\n", job_pid);
		return -1;
	}
	
	else { // check if the process is running in the backgroud.
		if (selected->active == 0) {
			printf("No process with pid %s running in backgroud.\n", cmd_tokens[1]);
			return -1;
		}
		else {
			int pgid, status, pid = selected->pid;
			pgid = getpgid(pid);
			tcsetpgrp(STDIN, pgid); // hey you come back
			
			if (killpg(pgid, SIGCONT) < 0)
				printf("Couldn't resume process with pid %d\n", pid);
				
			waitpid(pid, &status, WUNTRACED); 
			
			if(!WIFSTOPPED(status)) {
				selected->active = 0;
			}
			
			tcsetpgrp(STDIN, my_pid); // goodbye.
			
		}
		
	}
}

int exit_internal() { // exits cookie_shell
	
	free_history();
	token_cleanup(cmd_tokens, cmd_count);
	exit(0);
	
}


int history_internal() { // prints history
	if(get_head() == NULL) {
		printf("No commands in history.\n");
		return -1;
	}
	else {
		int size = size_history();
		command_node* head = get_head();
		int i = head->staticId;
			
		for (command_node* it = get_head(); it != NULL; it=it->next) {
			printf("[%d]\t", i+1);
			
			for(int j = 0; j < it->argc; j++)
				printf("%s ",it->data[j]);
				printf("\n");
				i++;
		}
	}
	return 0;
}

int help_internal() { // Simple help commands that prints internal available commands and their syntax.
	printf("\n Available internal commands:\n\n");
	printf("- help : \t \t Displays available commands.\n");
	printf("- exit : \t \t Exits cookie shell.\n");
	printf("- history : \t \t Displays command history.\n");
	printf("- ![id] : \t \t Executes command identified with history id [id].\n");
	printf("- jobs : \t \t Displays running jobs in backgroud.\n");
	printf("- fg [pid] : \t \t Brings backgroud job with pid [pid] to foregroud.\n");
	printf("- cd [directory] : \t Moves to [directory].\n");
	printf("- pwd : \t \t Displays current directory.\n");
	printf("- kill [pid] : \t \t Kills process with pid [pid].\n");
	printf("- [command] & : \t Execute [command] in the backgroud.\n\n");
	return 0;
}

int is_enph(char* command_tokens) {
	if (command_tokens[0] == '!')
		return 1;
	else 
		return 0;
}

// resolves links recursively in history ex, assume command 2 is ls: !7 -> !5 -> !2 -> ls => !7 -> ls. Returns node with ls command
// many checks and protections as an infinite loop is easy to get.
command_node *resolve_links(char** cmd_tk, int count) {
	
	if (count <= 0) { // Max depth. Prevents infinite loop in link resolving.
		return NULL;
	}
	
	
	// get command id
	char temp[sizeof(cmd_tk[0]-1)];
	strncpy(temp, &cmd_tk[0][1], sizeof(temp)); 
	int id = strtol(temp, NULL, 10);
	
	if (id <= 0) {
		return NULL;
	}
	
	// check if command id is in history.
	command_node* head = get_head();
	if ((id-1 < head->staticId) || (id-1 > get_tail()->staticId)) {
		return NULL;
	}
	
	
	// it is a normal command (external or internal but not of the form !id).
	if (is_enph(find_history(id-1)->data[0]) == 0) {
		return find_history(id-1);
	}
	
	// if looked up command is of the form !id then we must resolve link.
	else {
		
		char** tk_checked = find_history(id-1)->data;
		char checked[sizeof(tk_checked[0]-1)];
		strncpy(checked, &tk_checked[0][1], sizeof(checked));
		int id_c = strtol(checked, NULL, 10);
	
		if (id == id_c) { // Optimisation : Istead of hitting the stop recursion constant. Prevents infinite loop if user tries to load the command that loads the command.. :)
			return NULL;
		}
		
		// decrements stop recursion constant
		count--;
		return resolve_links(find_history(id-1)->data, count); // Everything is fine, resolve next history link
	}
	
	
}


// for !#
int enph_internal(char** command_tokens, int tk_count) {
	
	if (tk_count != 1) {
		printf("Invalid use of !. Usage ![command_id]\n");
		return -1;
	}
	
	command_node* selected = resolve_links(command_tokens, MAX_HISTORY_LINK_DEPTH);
	
	if (selected == NULL) {
		printf("No command found.\n"); // error message that can mean many different things...
		return -1;
	}

	// exec retreived command.
	cmd_dispatch(selected->data, selected->argc, selected->pipe, selected->redirect);
	
	return 0;
}

