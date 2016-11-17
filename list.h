
typedef struct _command_node {
    char **data; // commands
	int staticId; // real command number (!= position in linked list as linked list is limited by MAX_HISTORY_SIZE)
	int argc; // arg count
	int redirect; // 1 = internal command
	int pipe; // 1 = piped command
	int background; // backgroud command
	
    struct _command_node* next;
    
} command_node;
