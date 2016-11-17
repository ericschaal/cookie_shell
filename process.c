#include "headers.h"

/*
 * Keep track of processes. Processes are saved into an array.
 * If array is full an no process can be removed (all processes are active).
 * A process must be killed by the user.
 */

int available_process_index() {
	
	for (int i = 0; i < MAX_JOBS; i++) {
		if (process_table[i].active == 0) {
			process_table[i].active = 1; //keep a space for process that asked index.
			return i;
		}
	}
	
	return -1;
	
}

// be carefull with this... Used when pipe tries to lock two indexes but only one is available.
// The first that was locked needs to be released.
int free_proccess_index(int process_index) {
	
	// invalid index
	if ((process_index < 0) || (process_index >= MAX_JOBS))
		return -1;
	
	if (process_table[process_index].active) {
		process_table[process_index].active = 0;
		return 0;
	}
	else { // not locked.
		return -1;
	}
}


// creates a new process
int new_process(int pid, char* name, int idx) {
	
	process_table[idx].active = 1;
	process_table[idx].pid = pid;
	process_table[idx].name = strdup(name);
	
	return 0;
	
}


// finds a process identified by pid, in the process table.
process* find_process(int pid) {
	
	for (int i = 0; i < MAX_JOBS; i++) {
		if (process_table[i].pid == pid) {
			return &process_table[i];
		}
	}
	return NULL;
}

// removes a process from the process table.
int remove_process(int pid) {
	process* selected = find_process(pid);
	if (selected != NULL ) {
		selected->active = 0;
		selected->pid = 0;
		if (selected->name)
			free(selected->name);
		selected->name = NULL;
		return 0;
	}
	return -1;
}
