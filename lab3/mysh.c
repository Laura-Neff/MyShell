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

pid_t kill_list[1024];

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

        int error = 0;

        fprintf(stdout, "Here lies my hopes and dreams:: ");

        char *linebuf = malloc(sizeof(char) * (MYSH_LINE));
        if(!linebuf) {
            fprintf(stderr, "Error: Out of memory\n");
            exit(-1);
        }

        char * result = fgets(linebuf, MYSH_LINE, stdin); //reads one line from input
        //fgets returns 0 when it's at the end of a file or there was an error
        //fgets(string being read and stored (linebuf is being stored from stdin, max number of characters to be read, file stream that identifies where characters are read from)

        if((!result)||(strcmp(linebuf, "exit\n")==0)){ //if we got ctrl-D as input (EOF), or user typed in exit, quit
            fprintf(stdout, "Goodbye!\n");
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
            //fprintf(stdout, "My tokens: %s\n", breakup[i]);
            if(strcmp(breakup[i], "&") == 0) {
                if(breakup[i+1] != NULL) {
                    fprintf(stderr, "Error: background must be last character\n");
                    error=1;
                    break;
                }
                else {
                    //fprintf(stdout, "Place commands into the background: after invoking the specified commands.‘&’ may only be specified as the final command line argument. %s\n", breakup[i]);
                    commandSet->flag=1;
                }
            } else if(strcmp(breakup[i], "<") == 0) {
                    //fprintf(stdout, "Redirect the current command’s standard input stream from the file named immediately after the ‘<’ operator.\n");
                    i++; //increment i to the next token
                    //fprintf(stdout, "%s is our new input stream\n", breakup[i]);
                    if (command->inputFile){
                        fprintf(stderr,"Error: Input already specified.\n");
                        error=1;
                        break;
                    }
                    if (breakup[i]){
                        command->inputFile = breakup[i];
                        //fprintf(stdout,"inputting from: %s\n",new_command->input);
                    } else {
                        fprintf(stderr,"Error: No input specified.\n");
                        error=1;
                        break;
                    }

            } else if(strcmp(breakup[i], ">") == 0) {
                    //fprintf(stdout, "Redirect the current command’s standard output stream to the file named immediately after the ‘>’ operator.\n");
                    i++; //increment i to the next token
                    //fprintf(stdout, "%s is our new output stream\n", breakup[i]);
                    if (command->outputFile){
                        fprintf(stderr,"Error: Output already specified.\n");
                        error=1;
                        break;
                    }
                    if (breakup[i]){
                        struct stat buf;
                        int exists = lstat(breakup[i],&buf);
                        if (exists >= 0){
                            fprintf(stderr, "Error: output file exists.\n");
                            error = 1;
                            break;
                        }
                        command->outputFile = breakup[i];
                        //fprintf(stdout,"inputting from: %s\n",new_command->input);
                    } else {
                        fprintf(stderr,"Error: No output specified.\n");
                        error=1;
                        break;
                    }
            } else if(strcmp(breakup[i], ">>") == 0) {
                    //fprintf(stdout, "Redirect the current command’s standard output stream to the file named immediately after the ‘>>’ operator. Append to the file if it already exists.\n");
                    i++;
                    //fprintf(stdout, "%s is our new SPECIAL output stream\n", breakup[i]);
                    if (command->outputFile){
                        fprintf(stderr,"Error: Output already specified.\n");
                        error=1;
                        break;
                    }
                    if (breakup[i]){
                        command->outputFile = breakup[i];
                        command->appendOut = 1;
                        //fprintf(stdout,"inputting from: %s\n",new_command->input);
                    } else {
                        fprintf(stderr,"Error: No output specified.\n");
                        error=1;
                        break;
                    }

            } else if(strcmp(breakup[i], "|") == 0) {
                    //fprintf(stdout, "Redirect the current command’s standard output stream to the standard input stream of the succeeding command. There may be any number of pipe-connected processes..\n");
                    //fprintf(stdout, "%s is the first command's output stream to be used for the standard input stream of the succeeding command\n", breakup[i-1]);
                    //fprintf(stdout, "%s is our command that will be using the other command's output as an input\n", breakup[i+1]);
                    if (!command->arguments){
                        fprintf(stderr,"Error: No program to pipe.\n");
                        error=1;
                        break;
                    }
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
                    //fprintf(stdout, "Here is the command.\n");
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
        /*
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
        */

    for(int c = 0; commandSet->commands[c]; c++) {
        cmd command = commandSet->commands[c];
        if(command->pipeOutput){ //this means that this will pipe out
            int pipefds[2] = {-1,-1}; //init new array for pipe file descriptors
            if(!command->arguments[0]) {
                fprintf(stderr,"Error: No source for pipe.\n");
                error=1;
                break;
            } else if(command->outputFile){
                fprintf(stderr,"Error: Cannot redirect to file and pipe simultaneously.\n");
                error=1;
                break;
            } else {
                if(pipe(pipefds)<0){
                    perror("Error: Cannot create pipe()");
                    error=1;
                    break;
                }
                command->pipeOutput = pipefds[1]; //set pipe it is writing to
            }
            if (commandSet->commands[c+1]){ //if there is a next command
                if(!commandSet->commands[c+1]->pipeInput){ //if the next command's pipeinput is not set, should not happen
                    fprintf(stderr,"Error: Destination not set to accept pipe. Additionally, commands not parsed correctly!\n");
                    error=1;
                    break;
                } else if (!commandSet->commands[c+1]->arguments[0]){
                    fprintf(stderr,"Error: No destination for pipe.\n");
                    error=1;
                    break;
                } else if (commandSet->commands[c+1]->inputFile){
                    fprintf(stderr,"Error: Cannot redirect from input file and pipe simultaneously.\n");
                    error=1;
                    break;
                } else {
                    commandSet->commands[c+1]->pipeInput = pipefds[0]; //set pipe it is reading from 
                }
            } else { //there is no next command, should not happen
                fprintf(stderr,"Error: No destination for pipe. Additionally, commands not parsed correctly!\n");
                error=1;
                break;
            }
        }

    }
    if (error) continue; //just bring back the prompt if there was an error anywhere in the process

    // pid_t id = fork();
    int stream;
    // int pipefd[2] = {-1, -1};
    // int nbytes, g, line;
    // char is[MYSH_LINE];
    // char os[MYSH_LINE];
    pid_t process_id = 0;
    
    for(int c = 0; commandSet->commands[c]; c++) {
        cmd command = commandSet->commands[c];
        pid_t pid = fork();
        if(pid > 0) { //in the parent
            if(command->pipeOutput) { //close parent's writing pipe after fork
                close(command->pipeOutput);
            }
            if(command->pipeInput) { //close parent's reading pipe after fork
                close(command->pipeInput);
            }

        } else if (pid == 0){ //in the child
            // execvp(command->arguments[0], command->arguments);
            //fprintf(stdout,"Preparing command %s\n",command->arguments[0]);
            if(command->inputFile) {
                int fin = open(command->inputFile,O_RDONLY); 
                if(fin==-1){
                    perror("Error: input redirection");
                    exit(-1);
                }
                if(dup2(fin,0)==-1){ //redirect stdin
                    perror("Error: dup2");
                    exit(-1);
                }
                // printf("close(%d) ...\n", stream);
                // close(inputStream);
            }
            if(command->outputFile){
                int fout;
                if(command->appendOut){ //if we are just appending
                    fout = open(command->outputFile,
                                O_WRONLY|O_CREAT|O_APPEND, 
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //redirect in child!!
                } else { //if we are creating a new file or exiting if one found
                    fout = open(command->outputFile,
                                O_WRONLY|O_CREAT,
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                }
                if(fout==-1){
                    perror("Error: open()");
                    exit(-1);
                }
                if(dup2(fout,1)==-1){ //redirect stdout
                    perror("Error: dup2()");
                    exit(-1);
                } 
            }
            if(command->pipeOutput){
                if(dup2(command->pipeOutput, 1) == -1) {
                    fprintf(stderr, "Error dup2 stdout, pipeOutput %d\n",command->pipeOutput);
                    exit(1);
                }
            }
            if(command->pipeInput){
                if(dup2(command->pipeInput, 0) == -1) {
                    fprintf(stderr, "Error dup2 stdin, pipeInput %d\n",command->pipeInput);
                    exit(1);
                }
            }
            //fprintf(stderr,"Running command %s\n",command->arguments[0]);
            execvp(command->arguments[0], command->arguments);
            exit(-1);
        } else {
            fprintf(stderr,"Error: fork()\n");
            exit(-1);
            //Fork Error
            }
        }

        int status;
        pid_t id;
        do {
            do {
                id = wait(&status);
                //fprintf(stdout,"Waited on %d\n",id);
            } while(!WIFEXITED(status) && !WIFSIGNALED(status));
        } while(id != -1);

    

    free(linebuf);
    free_tokens(breakup);

    for (int i=0; commandSet->commands[i]; i++){
        command = commandSet->commands[i];
        if(command->arguments) free(command->arguments);
        //if(command->inputFile) free(command->inputFile);
        //if(command->outputFile) free(command->outputFile);
        free(command);
    }
    free(commandSet);

    }

}
