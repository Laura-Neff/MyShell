// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - LAURA NEFF

#include <stdio.h> //file i/o commands
#include <sys/stat.h> //checking file types of file descriptors
#include <sys/types.h> //fstat, other file i/o, wait
#include <sys/time.h> //for wait
#include <sys/resource.h> //for wait
#include <sys/wait.h> //for eait
#include <getopt.h> //for getopt
#include <unistd.h> //pipe command, parsing command line stuff, pause, forking
#include <stdlib.h> //malloc and stuff
#include <errno.h> //for the errno checking
#include <fcntl.h> //for O_CONSTANTS
#include <string.h> //fprintf, etc.

#include "tokens.h" //custom tokenizer made for the class



#define MYSH_LINE 1024



typedef struct CmdSet {
    struct Cmd ** commands;
    int flag;
    int numCmds;

} * cmdset;

 typedef struct Cmd {
    char** arguments;
    char* inputFile;
    char* outputFile;
    int numArgs;
    int appendOut;
    int pipeInput;
    int pipeOutput;


} * cmd;



int main( int argc, char *argv[] )
{
    while(1) {
        // commands = getCmd();

        // foreach command in commands: create succeeding pipe for I/O*
        // create child process open files for I/O*
        // redirect stdin from file or preceding pipe*
        // redirect stdoutto file or succeeding*
        // execute command in child
        // track childpid in parent
        // parent waits for foreground children*

        fprintf(stdout, "Here lies my hopes and dreams: ");

        char *linebuf = malloc(sizeof(char) * (MYSH_LINE));
        if(!linebuf) {
            fprintf(stderr, "Out of memory\n");
        }

        char * result = fgets(linebuf, MYSH_LINE, stdin); //reads one line from input
        //fgets returns 0 when it's at the end of a file or there was an error
        //fgets(string being read and stored (linebuf is being stored from stdin, max number of characters to be read, file stream that identifies where characters are read from)

        if(!result){
            fprintf(stdout, "Read line: %s", linebuf);
            return 0;
        }


        cmdset commandSet = calloc(1, sizeof(struct CmdSet));
        commandSet->numCmds = 1;
        commandSet->commands = calloc(1, sizeof(char*) * (commandSet->numCmds + 1));
        cmd command = calloc(1, sizeof(struct Cmd));
        command->arguments = calloc(1, sizeof(char **));
        commandSet->commands[0] = command;
        commandSet->commands[1] = NULL;
        


       char** breakup = get_tokens(linebuf);

   

        for( int i=0; breakup[i]; i++ ) {
            fprintf(stdout, "My tokens: %s\n", breakup[i]);
            if(strcmp(breakup[i], "&") == 0) {
                if(breakup[i+1] != NULL) {
                    fprintf(stdout, "This is not the final command line argument, you bozo.\n");
                }
                else {
                    fprintf(stdout, "Place commands into the background: after invoking the specified commands.‘&’ may only be specified as the final command line argument. %s\n", breakup[i]);
                    commandSet->flag=1;
                }
            }
             else if(strcmp(breakup[i], "<") == 0) {
                    fprintf(stdout, "Redirect the current command’s standard input stream from the file named immediately after the ‘<’ operator.\n");
                    i++;
                    fprintf(stdout, "%s is our new input stream\n", breakup[i]);
                    command->inputFile = breakup[i];

                    // FILE * inputStream = fopen(breakup[i+1], "r");
                    // fgetc(inputStream);
                    // fclose(inputStream);

                }
            else if(strcmp(breakup[i], ">") == 0) {
                    fprintf(stdout, "Redirect the current command’s standard output stream to the file named immediately after the ‘>’ operator.\n");
                    i++;
                    fprintf(stdout, "%s is our new output stream\n", breakup[i]);
                    command->outputFile = breakup[i];

                    // FILE * inputStream = fopen(breakup[i+1], "w");
                    // fgetc(inputStream);
                    // fclose(inputStream);
                }
            else if(strcmp(breakup[i], ">>") == 0) {
                    fprintf(stdout, "Redirect the current command’s standard output stream to the file named immediately after the ‘>>’ operator. Append to the file if it already exists.\n");
                    i++;
                    fprintf(stdout, "%s is our new SPECIAL output stream\n", breakup[i]);
                    command->inputFile = breakup[i];
                    command->appendOut = 1;

                }
            else if(strcmp(breakup[i], "|") == 0) {
                    fprintf(stdout, "Redirect the current command’s standard output stream to the standard input stream of the succeeding command. There may be any number of pipe-connected processes..\n");
                    fprintf(stdout, "%s is the first command's output stream to be used for the standard input stream of the succeeding command\n", breakup[i-1]);
                    fprintf(stdout, "%s is our command that will be using the other command's output as an input\n", breakup[i+1]);

                    commandSet->numCmds++;
                    commandSet->commands = realloc(commandSet->commands, sizeof(char*) * commandSet->numCmds + 1);
                    command->pipeOutput = 1;
                    cmd command2 = calloc(1, sizeof(struct Cmd));
                    command2->pipeInput = 1;
                    command2->arguments = calloc(1, sizeof(char **) * 2);
                    commandSet->commands[commandSet->numCmds - 1] = command2;
                    command = command2;
                    commandSet->commands[commandSet->numCmds] = NULL;

            } else {
                    fprintf(stdout, "Here is the command.\n");
                    if(command->numArgs == 0) {
                        command->arguments[0] = breakup[i];
                        command->numArgs++;
                    } else {
                        command->numArgs++;
                        command->arguments = (char**) realloc(command->arguments, sizeof(char**) * (command->numArgs+1));
                        command->arguments[command->numArgs - 1] = breakup[i];
                        command->arguments[command->numArgs] = NULL;
                     }


                }

        }

        for(int j=0; commandSet->commands[j]; j++){
            fprintf(stdout,"Command #%d, ",j);
            cmd out_command = commandSet->commands[j];
            fprintf(stdout,"vector: ");
            for (int k=0; out_command->arguments[k];k++){
                fprintf(stdout,"%s ",out_command->arguments[k]);
            }
            if (out_command->inputFile){
                fprintf(stdout,", input file: %s, ",out_command->inputFile);
            } else {
                if(out_command->pipeInput){
                    fprintf(stdout,", input file: pipe, ");
                } else {
                    fprintf(stdout,", input file: stdin, ");
                }
            }
            if (out_command->outputFile){
                fprintf(stdout,"output file: %s",out_command->outputFile);
                if(out_command->appendOut){
                    fprintf(stdout," (append), ");
                } else {
                    fprintf(stdout," (new file), ");
                }
            } else {
                if(out_command->pipeOutput){
                    fprintf(stdout,"output file: pipe, ");
                } else {
                    fprintf(stdout,"output file: stdout, ");
                }
            }
            if (commandSet->flag){
            fprintf(stdout,"background\n");
        } 
            else {
            fprintf(stdout, "foreground\n");
        }

        }

        pid_t id = fork();
        if(id > 0) {
            int status;
            pid_t child_process_id = wait(&status); //status will be updated through wait method with what happened to the child after it finished running
            if(child_process_id == -1) {
                perror("Wait didn't work.");
            }
            }
        else if (id == 0){
            execvp(command->arguments[0], command->arguments);
            exit(-1);
        }
        else {
            exit(-1);
            //Fork Error
        }
        printf("%d fork() returned %d. Parent's pid: %d\n", getpid(), id, getppid());
      





        free(linebuf);




    }

}
