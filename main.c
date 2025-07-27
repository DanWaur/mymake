/*
main.c
by Daniel Reynaldo

Given a filename as a command-line argument, whose file contents contain
make targets, dependencies, and commands. This utilizes "graph.c" to 
contstruct an adjacency list which stores the nodes and their dependencies
and targets. The second command line argument is a target name, and a post order
traversal is used to rebuild targets if needed.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "graph.h"


// Malloced variables
char *buff;
char *line;
char *targetName;
struct node *nodes;
FILE *toRead;


/* Frees all memory from main and the graph */
void freeAll(){
    free(buff);
    free(line);
    free(targetName);
    fclose(toRead);
    freeGraph(nodes);
}


/* This function returns 1 if a string is blank, 0 otherwise.
'line' is the string to check.
*/
int lineBlank(char *line){
    char *rest = line;
    while (*rest != '\0'){
        if (!isspace(*rest)){
            return 0;
        }
        rest++;
    }

    return 1;
}


/* This function returns 1 if a string has a ':', 0 otherwise.
'word' is the input string to check
*/
int hasColon(char *word){
    int i = 0;
    while (word[i] != '\0'){
        if (word[i] == ':'){
            return 1;
        }
        i++;
    }

    return 0;
}

int main(int argc, char *argv[]){

    // Adjacency List
    nodes = NULL;

    // Check parameter count
    if (argc > 4){
        fprintf(stderr, "Error: Program not run with expecteded parameters: [progName -f sourceFileName aTarget]\n");
        
        exit(1);
    }

    int fParam = 0; //boolean
    int targetIndex = -1;
    int fileIndex = -1;

    // Find arguments
    int curArg = 1;
    while (curArg < argc){

        // File parameter found, next will be filename, so set boolean to 1. if already one, more than one -f param so error
        if (strcmp(argv[curArg], "-f") == 0){
            if (fParam >= 1) {
                fprintf(stderr, "Error: More than one '-f' parameter detected\n");
        
                exit(1);
            }
            fParam = 1;
        }

        // boolean is true, treat argument as file name and set boolean to 2 to mark as already read
        else if (fParam == 1){
            fileIndex = curArg;
            fParam = 2;
        }
        else{
            targetIndex = curArg;
        }

        curArg++;
    }

    // No file parameter and 3 or more parameters
    if (fParam == 0 && argc >= 3){
            fprintf(stderr, "Error: Too many parameters\n");
    
            exit(1);
    }

    if (fileIndex == -1 && fParam > 0){
        fprintf(stderr, "Error: '-f' parameter supplied but no filename given\n");

        exit(1);
        
    }

    if (fileIndex > -1){
        // Read file
        toRead = fopen(argv[fileIndex], "r");
        if (toRead == NULL){
            fprintf(stderr, "File not found!\n");
            exit(1);
        }
    }
    else{
        toRead = fopen("myMakefile", "r");
        if (toRead == NULL){
            fprintf(stderr, "Default file 'myMakefile' not found!\n");
            exit(1);
        }
    }






    // Buffer for scanned words
    buff = malloc(65*sizeof(char));
    if (buff == NULL){
        fprintf(stderr, "OUT OF MEMORY!!!\n");
        fclose(toRead);
        exit(1);
    }


    targetName = malloc(65*sizeof(char));
    int firstFound = 0;

    line = NULL;
    char *rest = NULL;
    int offset;
    size_t num;

    // Last scanned targ
    node *targ = NULL;
    int retVal = getline(&line, &num, toRead);
    while ( retVal >= 0){
        offset = 0;
        rest = line;

        // Command line
        if (*line == '\t'){
            if (targ == NULL){
                fprintf(stderr, "Command found before target identified\n");
                freeAll();
                exit(1);
            }

            newCommand(targ, line+1, nodes);

        }
        // if line is blank, ignore
        else if (lineBlank(line)){

        }
        // Target : dependencies line
        else{


            // Scan first target, add to offset for next scan. if empty, skip line.
            char current = *rest;

            // Skip trailing whitespace
            while (isspace(current)){
                rest++;
                current = *rest;
            }

            // Scan up until next white space or colon, store in buff (target)
            int bufI = 0;
            while (!isspace(current) && current != ':' && bufI < 63){

                buff[bufI] = current;
                rest++;
                current = *rest;
                bufI++;
            }
            buff[bufI] = '\0';

            if (strlen(buff) == 0){
                fprintf(stderr, "Error: target not found\n");
                freeAll();
                exit(1);
            }

            // If a first target hasn't been found, copy name into targetName
            if (!firstFound){
                strcpy(targetName, buff);
                firstFound = 1;
            }


            if (bufI > 0){
                targ = addNode(buff, 1, &nodes);
                if (targ == NULL){
                    fprintf(stderr, "Error: target %s is already a target\n", buff);
                    freeAll();
                    exit(1);
                }

                // Skip trailing whitespace
                while (isspace(current) && current != '\0'){
                    rest++;
                    current = *rest;
                }
                
                if (current != ':'){
                    fprintf(stderr, "Error: no ':' between target and dependencies\n");
                    freeAll();
                    exit(1);
                }

                // Colon detected, continue
                else{
                    rest++;
                }

                // Scan rest, each is dependency.
                int retMore = sscanf(rest, "%64s%n", buff, &offset);
                while (retMore > 0){
                    if (hasColon(buff)){
                        fprintf(stderr, "Error: Too many ':' in dependency line\n");
                        freeAll();
                        exit(1);
                    }

                    newDep(targ, buff, nodes);
                    rest+= offset;
                    retMore = sscanf(rest, "%64s%n", buff, &offset);
                }
                

            }
        }

        retVal = getline(&line, &num, toRead);
    }

    // If target specified, used for name
    if (targetIndex > -1){
        strcpy(targetName, argv[targetIndex]);
    }
    //Otherwise, the first target was already placed

    if(findTarg(targetName, nodes) == NULL){
        fprintf(stderr, "Error: Target not specified in graph\n");
        freeAll();
        exit(1);
    }



    printPostOrder(targetName, nodes);

    if (!commandsWereRun()){
        printf("%s is up to date.\n", targetName);
    }


    freeAll();

    return 0;
    

}

    