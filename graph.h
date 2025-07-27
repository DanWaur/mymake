/*
graph.h
by Daniel Reynaldo

This defines the structures and public methods to be used by graph.c 
and main.c
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <sys/stat.h>

struct dep;
struct command;
struct node;

typedef struct node
{
    int isTarget;
    int visited;
    char *name;
    struct node *next;
    struct dep *deps;
    struct command *commands;
    int mustBuild;
    int completed;

    int exists;
    time_t timeS;
    time_t timeN;
    

} node;

typedef struct dep
{
    struct node *to;
    struct dep *next;
 
} dep;

typedef struct command{
    char *name;
    struct command *next;
} command;


void printNodes(node *nodes);

void newDep(node *from, char *nameTo, node *nodes);

node *addNode(char *name, int isTarg, node **nodes);

void newCommand(node *node, char *com, struct node *nodes);

void printPostOrder(char *nameStart, node *nodes);

node *findTarg(char *name, node *nodes);

void freeGraph(node *nodes);

int commandsWereRun();


//main

void freeAll();


#endif