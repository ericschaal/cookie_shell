#include "headers.h"


void set_vars() {
	getcwd(base_dir, MAX_BUFFER_SIZE-1);
	strcpy(pwd, base_dir);
	cmd_count = 0;
	ln_count = 0;
	cmd_tokens = NULL;
	
	my_pid = my_pgid = getpid();
	setpgid(my_pid, my_pgid);
	tcsetpgrp(STDIN, my_pgid);
}

void init() {
	
	reset_command();
	
	
	// Ignore these
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN); 
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);

	set_vars();
	pwd_rw();
	
}

// reset at each user input
void reset_command() {
	cmd_count = 0;
	output_redirect = 0;
	piping = 0;
	bg_exec = 0;
	
}

void signal_handler(int signum) {
        if(signum == SIGINT) { // CTRL-C management
				printf("\nCTRL+C received. Exiting with status 2.\n");
				exit(2);     
        }
        else if(signum == SIGCHLD) {            // for child signal mngmt
                int i, status, die_pid;
                while((die_pid = waitpid(-1, &status, WNOHANG)) > 0) {  //Get id of the process which has terminated .
                        for(i = 0; i < MAX_JOBS; i++)
                        {
                                if(process_table[i].active==0) // Although process is terminated it is still labeled as active in my process table. Thus if innactive no need to proccess it.
									continue;
                                else if(process_table[i].pid == die_pid) // found him. (terminated and not updated to innactive in my process table)
                                        break;
                        }
						if (i < MAX_JOBS) {
							if (VERBOSE)
								printf("\n%s with pid %d has exited with status %d \n", process_table[i].name, process_table[i].pid, WIFSIGNALED(status));
							process_table[i].active = 0; // label process as innactive.
							process_table[i].pid = -1;
							free(process_table[i].name);
						}
						
                }
        }
}

