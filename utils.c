#include "headers.h"

// free history to keep history <= MAX_HISTORY_SIZE
void history_cleanup() {
	if (current_history_size > MAX_HISTORY_SIZE) {
		remove_history();
	}
}

// free pipe data structures
void pipe_token_cleanup() {
	
	if (parent_args && child_args) {
		
		for (int i = 0; i < MAX_ARGS; i++) {
			free(child_args[i]);
			child_args[i] = NULL;
		}

		for (int i = 0; i < MAX_ARGS; i++) {
			free(parent_args[i]);
			parent_args[i] = NULL;
		}
		
	}
	
}

// free all tokens.
void token_cleanup(char** tokens, int tk_count) {
	int i = 0;
	if (cmd_tokens) {
		
		for (int i = 0; i < tk_count; i++) {
			free(tokens[i]);
		}
		
		free(tokens);
	}
}


// beautiful rewrite of path (displays ~ for the home directory instead of /xx/yy/zzz/qqq...)
// What is home directory ???? The path from where cookie is lunched.
void pwd_rw() {
	if ((strcmp(pwd, base_dir) == 0)) {
		strcpy(short_pwd, "~/");
	}
	else {
		strcpy(short_pwd, pwd);
	}
}

