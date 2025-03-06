#include "uart.h"
#include "mbox.h"
#include "function.h"
#include "cli_function.h"



// Function for backspace
//--------------------------------------------------------------------------------
void backspace(char* cli_buffer, int* index){
	(*index)--;						// Roll back
	cli_buffer[*index] = '\0';		// Pull null character to the previous index
	uart_sendc(EMPTY_CHAR);			// Send empty character
	uart_sendc(BACKSPACE);			// Send backspace character
}
// Function to clear the cli
//--------------------------------------------------------------------------------
void clear_cli_line(char* cli_buffer, int* index) {
    while (*index > 0) {
        uart_sendc(BACKSPACE);
        backspace(cli_buffer, index);
    }
}
// Function to copy buffer
//--------------------------------------------------------------------------------
void copy_cli_buffer(char* cli_buffer, char* temp){
	while (*cli_buffer != '\0'){
		*temp = *cli_buffer;
		
		// Move to next character
		cli_buffer++; 
		temp++;
	}
	*temp= '\0';
}
// Function to record the command name to array
//--------------------------------------------------------------------------------
void record_buffer(char* cli_buffer, char* temp, int* index){
	while (*cli_buffer != '\0'){
		*temp = *cli_buffer;
		
		cli_buffer++; 
		temp++;
		(*index)++;
	}
	*temp= '\0';
}
// Function to remove whitespaces 
//--------------------------------------------------------------------------------
void remove_whitespace(char *str) {
    int i, j;

    // Remove leading whitespaces in the line
	for(i = 0; str[i] ==' ' || str[i] == '\t'; i++);
		
	for(j = 0; str[i]; i++) {
		str[j++] = str[i];
	}
	str[j] = '\0';
}

