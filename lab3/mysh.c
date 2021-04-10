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
    char** flags;

} cmdset;

 typedef struct Cmd {
    char** argv;
    char* inputFile;
    char* outputFile;


} cmd;



int main( int argc, char *argv[] )
{
    while(1) {
        // commands = getCmd();
        // foreach command in commands: create succeeding pipe for I/O*
        // create child processopen files for I/O*
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



       char** breakup = get_tokens(linebuf);

       int num_tokens = sizeof(breakup)/sizeof(breakup[0]);

        for(int a=0; a < num_tokens; a++){
            fprintf(stdout, "My tokens: %s", breakup[a]);
        }


    



        free(linebuf);




    }

};
