// Function for auto completion
void TAB(char* cli_buffer, int index);
// Function to check history via entering "_" or "+"
void history(char* cli_buffer, int* index, char* history_index);



// Character define
#define EMPTY_CHAR 32
#define BACKSPACE 8

// Function for backspace
void backspace(char* cli_buffer, int* index);
// Function to clear the cli
void clear_cli_line(char* cli_buffer, int* index);
//Function to copy buffer -
void copy_cli_buffer(char* cli_buffer, char* temp);
// Function to record the command name to array
// void copyWithCount
void record_buffer(char* cli_buffer, char* address, int* index);
// Function to remove whitespaces
void remove_whitespace(char *str);
