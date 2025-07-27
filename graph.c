/*
graph.c
by Daniel Reynaldo

This supplies functions for adding to an adacency list for
nodes, dependencies, and commands. It can also provide a post order
traversal of the graph to rebuild targets.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "graph.h"

int commandsRan = 0;
int errSeen = 0;

/* Returns 1 if any commands were run by the post order traversal*/
int commandsWereRun(){
    return commandsRan;
}

/*
Takes a linked list of dependencies, frees all of the 
dependencies in the linked list.
*/
static void freeDeps(dep *deps){
    dep *current = deps;
    dep *prev = current;

    while (current != NULL){
        prev = current;
        current = current->next;
        free(prev);
    }
}


/*
Takes a linked list of commands, frees all of the 
commands and command names in the linked list.
*/
static void freeCommands(command *coms){
    command *current = coms;
    command *prev = current;

    while (current != NULL){
        prev = current;
        current = current->next;
        free(prev->name);
        free(prev);
    }
}


/*
Takes a linked list of nodes. Iterates through each
node and calls freeCommands and freeDeps on that node. Then, frees
the current node name and node itself.
*/
void freeGraph(node *nodes){
    node *current = nodes;
    node *prev = current;

    while (current != NULL){
        freeCommands(current->commands);
        freeDeps(current->deps);
        prev = current;
        current = current->next;
        free(prev->name);
        free(prev);
    }


}


/*
name is the name of the new node to be added to the linked list: nodes.
isTarg is 0 if it is not a target, and 1 if it is a target. If the node
already exists in the linked list, it will return the Node. Otherwise the
node will be created and added and returned.

If a target is attempted to be added and the node as a target already exists,
it will return NULL.
*/
node *addNode(char *name, int isTarg, node **nodes){
    node *current = *nodes;
    node *prev = current;

    // If already in list, return found and set to isTarg
    while (current != NULL){
        if (strcmp(name, current->name) == 0){
            if (isTarg){
                
                // If target already exists, return null
                if (current->isTarget == 1){
                    return NULL;
                }
                current->isTarget = 1; 
            }
            return current;
        }

        prev = current;
        current = current->next;
    }

    // Construct node
    node *tmpnode;
    tmpnode = malloc(sizeof(struct node));
    if (tmpnode == NULL){
        fprintf(stderr, "OUT OF MEMORY!!!\n");
        freeAll();
        exit(1);
    }

    tmpnode->name = malloc(65*sizeof(char));
    if (tmpnode->name == NULL){
        fprintf(stderr, "OUT OF MEMORY!!!\n");
        freeAll();
        exit(1);
    }
    tmpnode->next = NULL;
    tmpnode->commands = NULL;
    tmpnode->deps = NULL;
    tmpnode->isTarget = isTarg;
    tmpnode->visited = 0;
    tmpnode->completed = 0;
    tmpnode->mustBuild = 0;
    tmpnode->exists = 0;
    tmpnode->timeS = 0;
    tmpnode->timeN = 0;
    strcpy(tmpnode->name, name);


    // Add as head of list
    if (*nodes == NULL){
        *nodes = tmpnode;
    }

    // Add to end of list
    else{
        prev->next = tmpnode;       
    }

    return tmpnode;
}


/*
Adds a command to a node's commands list. Iterates through com
linked list and adds to it.
*/
static void addCommand(node *node, command *com){
    // Add as head of list
    if(node->commands == NULL){
        node->commands = com;
        return;
    }


    // Add to end of Linked List
    command *current = node->commands;
    command *prev = current;
    while (current != NULL){
        prev = current;
        current = current->next;
    }

    prev->next = com;
}


/*
Takes a node, string, and a nodes linked list. Newline character is removed
from the string, and copied into a new allocated space, stored in a new command, which
is then added to the nodes linked list of commands.
*/
void newCommand(node *node, char *com, struct node *nodes){
    // Remove '\n' from command
    char *buff;
    buff = com;
    int length = 0;
    while(*buff != '\0'){
        if (*buff == '\n'){
            buff[0] = '\0';
        }

        length++;
        buff++;
    }

    char *newCom = malloc((length+1)*sizeof(char));
    if (newCom == NULL){
        fprintf(stderr, "OUT OF MEMORY!!!\n");
        freeAll();
        exit(1);
    }
    strcpy(newCom, com);


    // Construct dep
    command *tmpcom;
    tmpcom = malloc(sizeof(struct command));
    if (tmpcom == NULL){
        fprintf(stderr, "OUT OF MEMORY!!!\n");
        freeAll();
        exit(1);
    }
    tmpcom->next = NULL;
    tmpcom->name = newCom;

    addCommand(node, tmpcom);
}


/*
Adds a dependency to a nodes linked list of dependencies.
*/
static void addDep(struct node *node, struct dep *dep){
    // Add as head of list
    if(node->deps == NULL){
        node->deps = dep;
        return;
    }


    // Add to end of Linked List
    struct dep *current = node->deps;
    struct dep *prev = current;
    while (current != NULL){
        prev = current;
        current = current->next;
    }

    prev->next = dep;
}

/*
Creates a dependency using the target, and a string for the node it connects to. 
Adds the dependency to the nodes linked list.
*/
void newDep(node *from, char *nameTo, node *nodes){
    // Construct dep
    dep *tmpdep;
    tmpdep = malloc(sizeof(struct dep));
    if (tmpdep == NULL){
        fprintf(stderr, "OUT OF MEMORY!!!\n");
        freeAll();
        exit(1);
    }
    tmpdep->next = NULL;
    tmpdep->to = addNode(nameTo, 0, &nodes);

    addDep(from, tmpdep);
}

/*
Prints the commands associated with a node.
*/
static void printComs(node *node){
    command *current = node->commands;

    while (current != NULL){
        printf("  %s\n", current->name);
        current = current->next;
    }
}

/*
Finds a target by name in a linked list, nodes. returns the target
or NULL if not found.
*/
node *findTarg(char *name, node *nodes){
    node *current = nodes;

    while (current != NULL){
        if (strcmp(name, current->name) == 0 && current->isTarget){
            return current;
        }

        current = current->next;
    }

    return NULL;
}


/* uses stat() to determine the file read time and if it exists, sets these in the node
struct*/
static void setFileStatus(node *n){
    struct stat statistics;
    int noExist = stat(n->name, &statistics);
    n->exists = (noExist == 0);

    if (!n->exists){

    }
    else{
        n->timeS = statistics.st_mtimespec.tv_sec;
        n->timeN = statistics.st_mtimespec.tv_nsec;
    }

}

/* compares the dates of nodes a to b. Returns true
if a > b, false otherwise*/
static int compareDates(node *a, node *b){

    if (a->timeS == b->timeS){
        return a->timeN > b->timeN;
    }

    else{
        return a->timeS > b->timeS;
    }

}

/* Runs commands in a linked list of commands, if execution fails, it writes an
error*/
static void runCommands(command *com){

    commandsRan = 1;

    command *current = com;
    while(current != NULL){

        int executed = system(current->name);
        if (executed != 0){
            // REMEMBER TO FREE MEMORY
            fprintf(stderr, "Error: command failed to execute\n");
            freeAll();
            exit(1);
        }
        printf("%s\n", current->name);
        current = current->next;
    }
}


/*
Recursive post order function that goes from a start node to
next nodes, determines if target is up to date, checking dependencies
to reconstruct it.
*/
static void printPostOrderHelper(node *n){

    if (n->visited){
        return;
    }
    n->visited = 1;

    setFileStatus(n);
    //printf("Modify time of %s: %lld seconds, %lld nanoseconds\n", n->name, (long long) n->timeS, (long long) n->timeN);




    if (!n->exists){
        if (!n->isTarget){
            // REMEMBER TO FREE MEMORY
            fprintf(stderr, "Dependency does not exist\n");
            freeAll();
            exit(1);
        }
        else{
            n->mustBuild = 1;
        }
    }

    
    dep *d = n->deps;
    while(d != NULL){
        printPostOrderHelper(d->to);

        if (!d->to->completed){
            fprintf(stderr, "Error: cycle found\n");
        }
        else if (!n->mustBuild){
            if (!d->to->exists || compareDates(d->to, n)){
                n->mustBuild = 1;
            }
        }

        d = d->next;
    }


    if (n->mustBuild){
        runCommands(n->commands);
        setFileStatus(n);
    }

    n->completed = 1;

}

/*
Prints post order traversal using a starting target in a linked list of nodes.
*/
void printPostOrder(char *nameStart, node *nodes){
    node *start = findTarg(nameStart, nodes);

    printPostOrderHelper(start);

}



// FOR DEBUG
static void printDeps(node *node){
    dep *current = node->deps;

    while (current != NULL){
        printf("  Dependency to: %s\n", current->to->name);
        current = current->next;
    }
}

// FOR DEBUG
void printNodes(node *nodes){
    node *current = nodes;

    while (current != NULL){
        printf("NODE: %s\n", current->name);
        printComs(current);
        printDeps(current);
        current = current->next;
    }
}



