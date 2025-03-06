// -----------------------------------main.c -------------------------------------
#include "uart.h"
#include "function.h"
#include "cli_function.h"
#include "mbox.h"
#include "framebf.h"
#include "game.h"
#include "display_image.h"
#include "display_video.h"

#define MAX_CMD_SIZE 100
#define HISTORY_STORAGE 20
int count = 1;
int x_coordinate = 100;
int y_coordinate = 300;
int quitGame=0;
int restartGame=0;
int isStage2 = 0;
int check = 0;

// Declare cli() function
//--------------------------------------------------------------------------------
void cli()
{
	
	//read and send back each char
	char c = uart_getc();

	static char cli_buffer[MAX_CMD_SIZE];
	static int index = 0;
	// Array to record the past cli buffer
	static char cli_buffer_history[HISTORY_STORAGE][MAX_CMD_SIZE] = {"","","","","","","","","","",
																	 "","","","","","","","","",""};
	static int cli_buffer_history_index = 0;		// indexes for the history array
	static int history_next = 1; 					// get the next recorded cli buffer in history array
	int color_index = 0;			// Color index of cli
	
    Game game;

	// IF CLI get TAB
	if (c == '\t'){
		// TAB function
		// TAB(&cli_buffer, index);

		// STore current input cli to temp for comparison and clear current input cli
		char temp[100];
		copy_cli_buffer(cli_buffer, temp);			// Copy contents of cli_buffer to temp
		remove_whitespace(temp);					// Remove any whitespaces
		clear_cli_line(cli_buffer, &index);			// Clear the previous cli buffer line
		
		// Sweep through all 6 commands
        //--------------------------------------------------------
		if (comp_str(temp, "") == 0 || comp_str(temp, "0") == 0) {
			record_buffer("1", cli_buffer, &index);
			uart_puts(cli_buffer);
		} 
		else if (comp_str(temp, "1") == 0) {
			record_buffer("2", cli_buffer, &index);
			uart_puts(cli_buffer);
		} 
		else if (comp_str(temp, "2") == 0) {
			record_buffer("3", cli_buffer, &index);
			uart_puts(cli_buffer);
		} 
		else if (comp_str(temp, "3") == 0) {
			record_buffer("4", cli_buffer, &index);
			uart_puts(cli_buffer);			
		} 
		else if (comp_str(temp, "4") == 0) {
			record_buffer("5", cli_buffer, &index);
			uart_puts(cli_buffer);			
		} 
        else if (comp_str(temp, "5") == 0) {
			record_buffer("0", cli_buffer, &index);
			uart_puts(cli_buffer);			
		} 
	}
	// IF CLI in the current line
	//----------------------------------------------------------------------------------------
	else if (c == '_'){
		// clear line
		clear_cli_line(cli_buffer, &index);

		// Get the latest recorded cli buffer
		// Check if history storage is available
			if ((cli_buffer_history_index < HISTORY_STORAGE - 1) 
								&& 
					(comp_str(cli_buffer_history[cli_buffer_history_index + 1], "") != 0))
						{
							cli_buffer_history_index++;		// Move to the next index
							record_buffer(cli_buffer_history[cli_buffer_history_index], cli_buffer, &index);
							uart_puts(cli_buffer);
						}
			// Reaching the last recorded cli buffer
			else if (cli_buffer_history_index < HISTORY_STORAGE && history_next)
						{
							cli_buffer_history_index++;		// Move to the next index
							history_next = 0;
						}
	}
	// IF CLI in the current line
	//----------------------------------------------------------------------------------------
	else if (c == '+'){
		// clear line
		clear_cli_line(cli_buffer, &index);
		history_next = 1;

		// Get the latest recorded buffer cli
			if (cli_buffer_history_index > -1)
						{
							cli_buffer_history_index--;
							record_buffer(cli_buffer_history[cli_buffer_history_index], cli_buffer, &index);
							uart_puts(cli_buffer);
						}
			// Stop reading when reach the last stored cli_buffer
			else if (cli_buffer_history_index == -1)
						{
							history_next = -1; 
						}
	}
	// IF CLI in the current line
	//----------------------------------------------------------------------------------------
	else if (c != '\n'){
		uart_sendc(c);

		// If user enter backspace
		if (c == BACKSPACE) { // When pressing backspace
			if (index > 0) {
				backspace(cli_buffer, &index);
			} else {
				uart_sendc(EMPTY_CHAR);
			}
		} 
		else {
			cli_buffer[index] = c; //Store into the buffer
			index++;
		}

	} 
	// IF CLI receive any input data
	//----------------------------------------------------------------------------------------
	else if(c == '\n'){
		uart_sendc(c);
		cli_buffer[index] = '\0';
		// Update storage ofcommand history index
		cli_buffer_history_index = -1; history_next = 1;

		// Save command to history storage
		for (int i = HISTORY_STORAGE; i > 0; i--) {
			// Override the old cli_buffer with the new one and shift the old ones
			copy_cli_buffer(cli_buffer_history[i-1], cli_buffer_history[i]);
		}
		// Take the stored cli_buffer out
		copy_cli_buffer(cli_buffer, cli_buffer_history[0]);


		/* Compare with supported commands and execute
		* ........................................... */
		// Checking DISPLAY FONT COMMAND
		if (comp_str(cli_buffer, "3") == 0)
            {
                clearscreen(0, 0);
            
                font();
            }
		// Checking A SCROLLABLE LARGE IMAGE COMMAND
		else if(comp_str(cli_buffer,"1") == 0)
            {
				clearscreen(0,0);
            	controlScrollableImage();
                // Display Scrollable large images
            }
        // Checking A VIDEO COMMAND
		else if (comp_str(cli_buffer, "2") == 0)
            {
				clearscreen(0,0);
            	displayVideo(x_coordinate, y_coordinate);
                // Display video
            }
		// Checking if PLAY GAME COMMAND
		else if(comp_str(cli_buffer, "4") == 0)
            {
                clearscreen(0, 0);
                displayGameBackground(0, 0);
				init_game(&game);
				show_main_menu(&game);
				if (game.game_start)
				{
					displayGameUniverseBackground(0, 0);

					while (!quitGame)
					{
						displayGameUniverseBackground(0, 0);

						restart_game(&game);
						move_game(&game.world);
					}
				}
				clearscreen(0, 0);
            }
        // Checking CLEAR SCREEN COMMAND
		else if(comp_str(cli_buffer, "0") == 0)
            {
                clearscreen(0,0);
				uart_puts("\033[2J");  	// clear command screen
				uart_puts("\033[H");  	// scroll to top screen
            }
		else {
			uart_puts("Invalid command");
			uart_puts("\n Check the command menu and re-enter to get the correct syntax");
		}
		
		reset_arr(cli_buffer);
        displayMenu();
		uart_puts("\nAsm3_OS>: ");
		index = 0;
	}
}


void main()
{
    // set up serial console
    uart_init();
    // say hello
    // Initialize frame buffer
    framebf_init();

	// displayGameUniverseBackground(0, 0);

    // Print welcome interface
    displayMenu();

	uart_puts("\nAsm3_OS>: ");

    // Run CLI
    while (1)
    {  
        // Execute CLI
        cli();
    }
}