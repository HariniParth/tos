#include <kernel.h>

//Define the Shell Window
WINDOW shell_wnd = {0, 9, 61, 16, 0, 0, 0xDC};

#define MAX_CMD_LENGTH 20           //Define max length of a command
char input_buffer[MAX_CMD_LENGTH];  //Define an array to store the command being fed
int len_of_cmd;                     //Variable to store the length of the current command

/*
Function to compare two strings - One saved in Input Buffer (typed by the user)
and the other is the command we declare to support - 'ps', 'help' etc
*/
int str_compare(char str1[],char str2[])
{
    int i=0, flag=0;
    while(str1[i]!='\0' && str2[i]!='\0'){ //Until end of string is not reached
        if(str1[i]!=str2[i]){              //Check each character of the strings
            flag = 1;
            break;
        }
        i++;
    }
    int j=i+1;
    if(flag==0 && str1[i]=='\0' && str2[i]=='\0'){
        while(j<MAX_CMD_LENGTH){
            if(str1[j] == '\0')
                return 1;                  //Return 1 if both strings match
            else
                return 0;
        }
    } 
    else
        return 0;
}

/*
Function that displays the list of commands supported by TOS
*/

void help(){
    wprintf(&shell_wnd,"'help'    - Display the list of available commands\n");
    wprintf(&shell_wnd,"'ps'      - Display the list of processes\n");
    wprintf(&shell_wnd,"'about'   - Display information about this OS\n");
    wprintf(&shell_wnd,"'train'   - Initialize train application\n");
    wprintf(&shell_wnd,"'tgo'     - Start the train at speed 4\n");
    wprintf(&shell_wnd,"'tstop'   - Stop the running train\n");
    wprintf(&shell_wnd,"'reverse' - Reverse the direction of the train\n");
    wprintf(&shell_wnd,"'clear'   - Clear the screen\n");
    wprintf(&shell_wnd,"'tswitch' - Change the switch mode on the track\n");
    wprintf(&shell_wnd,"\n");
}

/*
Function that displays information about this OS
*/

void about()
{
    wprintf(&shell_wnd,"Train Operating System\nAdvanced Operating System - CSC 720\nHarini Parthasarathy - 915575302\n");
    wprintf(&shell_wnd,"\n");
}

/*
Function that nullifies the array (input_buffer - which stores the input command)
at the beginning of each line. 
*/

void clear_buffer()
{
    int i;
    for(i=0;i<MAX_CMD_LENGTH;i++){
        input_buffer[i]='\0';
    }
}

/*
Function which nullifies the 'space' characters succeeding a command
while ensuring that a non-space character typed after the spaces is retained.
Eg: This will allow 'ps  ' to work but not 'ps  ps'!
*/

void space_buffer()
{
    int i=0;
    for(i=0;i<len_of_cmd;i++){
        if(input_buffer[i] != (char)32){
            input_buffer[i]=input_buffer[i];
        } else
            input_buffer[i]='\0';
    }
}

/*
Function that executes commands based on what the user has typed (stored in input_buffer)
That is compared with the predefined list of commands - 'ps', 'help', 'about' etc. and 
the appropriate command is executed
*/

void execute_cmd()
{
    wprintf(&shell_wnd,"\n");
    if(str_compare(input_buffer,"ps") == 1){           //Print all the Processes
        print_all_processes(&shell_wnd);
        wprintf(&shell_wnd,"\n");
    } else if(str_compare(input_buffer,"help") == 1){  //Print all the available commands
        help();
    } else if(str_compare(input_buffer,"clear") == 1){ //Clear the screen
        clear_window(&shell_wnd);
    } else if(str_compare(input_buffer,"about") == 1){ //Display information about the OS
        about();
    } else if(len_of_cmd > 0){                         //For all other unsupported commands
        wprintf(&shell_wnd,"Command not found. \nType 'help' to get the list of available commands\n");
    }
    clear_buffer();
}

/*
Function accepting input characters and storing them in a buffer before printing out
appropriate information if the buffer stores a valid command
*/

void shell_process(PROCESS self, PARAM param)
{
    clear_buffer();
    wprintf(&shell_wnd,"tos>");
    len_of_cmd = 0;                     //Stores the length of the command, Initially set to 0
    char ch;
    Keyb_Message msg;
    while(1){
        msg.key_buffer = &ch;           //Accept and store input keyboard characters 
        send(keyb_port, &msg);
        if(ch == 13){                   //If 'Enter' is pressed
            space_buffer();             //Clear null characters succeeding the command
            execute_cmd();              //Execute the command
            wprintf(&shell_wnd,"tos>");
            len_of_cmd = 0;
        } else if(ch == '\b' | ch == 8){            //If 'Backspace' is pressed
            if(len_of_cmd > 0){                     //Check if length of command is > 0, i.e. Check if something is typed
                input_buffer[len_of_cmd-1] = '\0';  //Set the last typed character to null
                len_of_cmd--;                       //Reduce the length of the command
                wprintf(&shell_wnd, "%c", ch);
            }
        } else if(ch == 32 && len_of_cmd < MAX_CMD_LENGTH){ //If 'Space' ke is pressed
            if(len_of_cmd > 0){                 //Check if some character has been pressed before the 'Space' key
                wprintf(&shell_wnd, "%c", ch);
                input_buffer[len_of_cmd] = ch;  //If yes, add the 'Space' characters to the input buffer
                len_of_cmd++;
            } else {
                wprintf(&shell_wnd, "%c", ch);  //If no, disregard the preceeding 'Space' characters
            }
        } else if(len_of_cmd < MAX_CMD_LENGTH){ //For all other characters, display it on screen and store them in the input buffer
            wprintf(&shell_wnd, "%c", ch);
            input_buffer[len_of_cmd] = ch;
            len_of_cmd++;                       //Update the count of characters in the command
        }
    }
}

/*
Function to create a new Shell process
*/

void init_shell()
{
    clear_window(&shell_wnd);
    create_process(shell_process, 3, 0, "Shell process");
    resign();
}