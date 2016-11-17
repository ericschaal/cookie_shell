#include "headers.h"

char** parse_cmd() {
	
	int length;
	char *token, *loc;
	char *line = NULL;
	size_t linecap = 0;
	
	size_t tokens_alloc = 1; // keeps track of what is currently allocated
    size_t tokens_used = 0; // keeps track of the current number of tokens 
	
	
	printf("[%s]%s", short_pwd, PROMPT); 
	length = getline(&line, &linecap, stdin); // read from stdin.

	
	if (length <= 0) { 
		exit(-1);
	}
	
	if((loc = strchr(line, '|')) != NULL) { // there is a PIPE !
		
		// Check if | is not at the beggining of a command or doesnt have a space before or after.
		if ((loc[1] != ' ') || (&loc == &line) || (loc[-1] != ' ')) {
			printf("Please add a space before and after pipe.\n");
			free(line);
			cmd_count = 0;
			return NULL;
		}
		piping = 1;
	}
	
		if((loc = strchr(line, '>')) != NULL) { // there is a redirection !
		// Check if | is not at the beggining of a command or doesnt have a space before or after.
		if ((loc[1] != ' ') || (&loc == &line) || (loc[-1] != ' ')) {
			printf("Please add a space before and after redirect.\n");
			free(line);
			cmd_count = 0;
			return NULL;
		}
		output_redirect = 1;
	}
	
	
	char **tokens = calloc(tokens_alloc, sizeof(char*)); // first allocation
	char *strtok_ctx;
	
	for (token = strtok_r(line, DELIMS, &strtok_ctx); token != NULL; token = strtok_r(NULL, DELIMS, &strtok_ctx)) { // get token, realloc if needed.
		
        // check if more memory is needed.
        if (tokens_used == tokens_alloc) {
            tokens_alloc += 1;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
		// copy token to token array.
        tokens[tokens_used++] = strdup(token);
		
		// check if there is not too many arguments.
		if (tokens_used > MAX_ARGS) {
			printf("Too many arguments.\n");
			free(line);
			free(token);
			token_cleanup(tokens, tokens_used);
			tokens_used = 0;
			return NULL;
		}
		
    }
	
	// empty command, must free.
	if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } 
	
	// add '\0' add the end or exec will complain :)
	else {
        tokens = realloc(tokens, (tokens_alloc+1) * sizeof(char*));
		tokens[tokens_alloc] = '\0';
    }
	
    cmd_count = tokens_used;
	strtok_ctx = NULL;
	loc= NULL;
	
	free(line);
	free(token);
	
	return tokens;
	
}




// parse a token array to two token arrays. Used for piping
int parsePipe(char **args, int tk_count) {
	
	int pipe_idx;
	int pipe_count = 0;
	// find location of |.
	for (int i = 0; i < tk_count; i++) {
		if (pipe_count > 1) 
			return -1;
		if ( strcmp(args[i], "|") == 0 ) {
			pipe_idx = i;
			pipe_count++;
		}
		
	}

	// Compute argument count
	parent_args_c = pipe_idx;
	child_args_c = tk_count - pipe_idx - 1;
	
	// create first array.
	for (int i = 0; i < parent_args_c; i++) {
		parent_args[i] = strdup(args[i]);
	}
	
	// create second array.
	int j = 0;
	for (int i = (pipe_idx + 1); i < tk_count; i++) {
		child_args[j] = strdup(args[i]);
		j++;
	}
	
	// That's for you exec().
	parent_args[parent_args_c] = '\0';
	child_args[child_args_c] = '\0';
	
	return 0;
}