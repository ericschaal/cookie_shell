#include "headers.h"

int main() {
	
	init();
	
	// main loop
	while(1) {
		
		// set everything to 0
		reset_command();
		
		
		// Catch signals. Custom handling.
		if(signal(SIGCHLD, signal_handler)==SIG_ERR)
			printf("Cannot catch SIGCHLD.\n");
			
		if(signal(SIGINT, signal_handler)==SIG_ERR)
			printf("Cannot catch SIGINT.\n");
		
		// parse user input
		cmd_tokens = parse_cmd();
		
		// if command is not empty
		if (cmd_count > 0) {
				cmd_dispatch(cmd_tokens, cmd_count, piping, output_redirect);
				token_cleanup(cmd_tokens, cmd_count); // free
		}
	}
	
	return 0;
}
