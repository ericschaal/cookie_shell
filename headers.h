// Big file... I don't know if it's the best practice in c, but I have seen
// a few C programers doing it this way...

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h> 
#include <signal.h>
#include <errno.h>


#include "list.h"

// My shell is called cookie.

/* ************ CONFIG ***************/
#define PROMPT "Cookie>>"
#define DELIMS  " \n\t" // For parser.
#define MAX_ARGS 20 // Max number of arguments in user command
#define MAX_JOBS 20 // Max jobs in process_table (keep it high.. or low if you want to test)
#define MAX_HISTORY_LINK_DEPTH 5 // Security to prevent infinite loop links. +/- MAX_HISTORY_SIZE/2 is a good value.
#define MAX_HISTORY_SIZE 10 // Max # commands to keep in history.
#define MAX_BUFFER_SIZE 1024 // Used for paths
#define VERBOSE 0 // Verbose to print signals from processes. #ugly



// DO NOT MODIFY BELLOW //

// to prevent stupid errors
#define STDOUT 1 
#define STDIN 0

// holds processes
struct process {
	int pid;
	char* name;
	int active;
};

typedef struct process process;

// Command Management;
char** cmd_tokens; // holds command

// States for a command.
int cmd_count;
int ln_count; // used for history
int output_redirect;
int piping;
int bg_exec;
int current_history_size;


//Piping utils
char *parent_args[MAX_ARGS]; // not the most memory efficient. But easier
char *child_args[MAX_ARGS]; // not the most memory efficient. But easier
int parent_args_c;
int child_args_c;


// Directories Management
char base_dir[MAX_BUFFER_SIZE];
char pwd[MAX_BUFFER_SIZE];
char short_pwd[MAX_BUFFER_SIZE];


// Process informations
pid_t my_pid, my_pgid;
process process_table[MAX_JOBS];



//***************** Functions ********************//

// Parsers
char** parse_cmd();
int parsePipe(char **args, int tk_count);

// Process backend
int available_process_index();
int free_proccess_index(int process_index);
int new_process(int pid, char* name, int idx);
process* find_process(int pid);
int remove_process(int pid);

// Internal commands
int history_internal();
int enph_internal(char** command_tokens, int tk_count);
int kill_internal(char** cmd_tokens, int tk_count);
int fg_internal(char **cmd_tokens, int tk_count);
int jobs_internal();
int exit_internal();
int cd_internal(char** cmd_tokens, char* pwd, char* base_dir);
int help_internal();
command_node *resolve_links(char** cmd_tk, int count);
int is_enph(char* command_tokens);

//Initialization
void set_vars();
void init();
void signal_handler(int signum);
void handle_ctlc(int dum);


// History backend
int add_history(char* s[], int givenId, int argc, int piping, int redirect);
int remove_history();
int size_history();
void print_history();
int free_history();
command_node* find_history(int givenId);
command_node* get_tail();
command_node* get_head(); 

// utils
void reset_command();
void history_cleanup();
void token_cleanup(char** tokens, int tk_count);
void pipe_token_cleanup();
void pwd_rw();

// execution
void cmd_dispatch(char **cmd_tokens, int tk_count, int pipe, int redirect);
int run_pipe(char** parent_args, char** child_args);
int run(char **cmd_tokens);
int run_redirect(char** full_cmd_tokens, int tk_count);