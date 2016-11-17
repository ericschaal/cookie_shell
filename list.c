#include "headers.h"

// Many thanks to TA x for his linked list, that I modified a little to implement my history.

command_node* command_head = NULL;
command_node* command_tail = NULL;


command_node* get_head() {
	return command_head;
}
command_node* get_tail() {
	return command_tail;
}

// Add a command to the list
int add_history(char* s[], int givenId, int argc, int piping, int redirect) {	
    if (command_head == NULL) {
        command_head = malloc(sizeof(command_node));
        command_tail = command_head;
    }
    else {
        command_tail->next = malloc(sizeof(command_node));
        command_tail = command_tail->next;
    }
	command_tail->data = malloc(sizeof(char*)*(argc+1));
	for (int i = 0; i < argc; i++) {
		command_tail->data[i] = strdup(s[i]);
	}
	command_tail->data[argc] = '\0';
	// Filling data
	command_tail->next = NULL;
	command_tail->argc = argc;
	command_tail->staticId = givenId; // this will be the real command number.
	command_tail->redirect = redirect;
	command_tail->pipe = piping;
	
	current_history_size++;
	
    return 0;
    
}


// Find command identified by givenID.
command_node* find_history(int givenId) {
    for (command_node* it = command_head; it != NULL; it=it->next) {
        if (givenId == it->staticId) {
            return it;
        }
    }
    return NULL;
}


// Print n last commands. 
void print_history() {
	
	if(command_head == NULL) {
		printf("No commands in history.\n");
	}
	else {
		int i = command_head->staticId;
			
		for (command_node* it = command_head; it != NULL; it=it->next) {
			printf("[%d]\t", i+1);
			
			for(int j = 0; j < it->argc; j++)
				printf("%s \n",it->data[j]);
				
			printf("\n");
			i++;
		}
	}
}


// Size of the list.
int size_history() {
	
	if (command_head == NULL)
		return -1;
	
	int i = 0;
	
	command_node* it = command_head;
	while(1) {
		i++;
		if (it->next == NULL)
			return i;
		it = it->next;
	}
	
	return 0;
}


// Cleanup function
int free_history() {
	if (command_head == NULL) {
		return -1;
	}
	
	for(command_node* it = command_head; it != NULL; ) {
		command_node* temp = it->next;
		for (int i = 0; i< it->argc; i++) {
			free(it->data[i]);
		}
		free(it->data);
		free(it);
		it = temp;
	}
	
	return 0;
	
}


// Remove first command of list.
int remove_history() {
	
	if (command_head == NULL) {
		return -1;
	}
	
	command_node* temp = command_head;
	command_head = command_head->next;
	
	
	
	for (int i = 0; i < temp->argc; i++) {
		free(temp->data[i]);
		temp->data[i] = NULL;
	}
	temp->argc=-1;
	temp->pipe=-1;
	temp->background=-1;
	temp->redirect=-1;
	temp->staticId=-1;
	free(temp->data);
	temp->next = NULL;
	free(temp);
	temp = NULL;
	
	current_history_size--;
	
	return 0;
	
}