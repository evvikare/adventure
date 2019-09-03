/*******************************************************************************
*
* buildroom.c
*
* -----------------
* EvVikare
* evvikare@protonmail.com
* CS344 -- Summer 2017
* Program 2 Assignment
* -----------------
*
* Generate 7 different room files, one room per file. 
* Files will be in a directory called "evvik.rooms.[PID]",
* where [PID] is the process ID of running instance of buildroom.c 
*
*******************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct Room{
	int id;
	char* name;
	char* type;
	int numConnections;
	struct Room* connections[6];	
}Room;

void initRooms(Room* rms[]);
void makeConnections(Room* rms[]);
bool isValidConnection(Room* a, Room* b);
bool isNewNeighbor(Room* a, Room* b);
bool isDifferentRoom(Room* a, Room* b); 
bool isAvailable(Room* b);
void connectRooms(Room* a, Room* b);
void createRoomFiles(Room* rms[]);

int main(){
	srand(time(NULL));
    Room* rooms[7];
    initRooms(rooms);
    makeConnections(rooms);
    createRoomFiles(rooms);
    
    for(int i = 0; i < 7; i++){
        free(rooms[i]);
    }
    
    return 0;
}

// Choose a unique name at random (helper for initRooms)
int getName(){
    static int nameTaken[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int name;
    
    while(true){
        name = rand() % 10;
        if(!nameTaken[name]){
            nameTaken[name] = 1;
            return name;
        }
    }
}

void initRooms(Room* rms[]){
    char* roomTypes[] = { "START_ROOM", "END_ROOM", "MID_ROOM" };
    char* roomNames[] = {
        "Gazorpazorp", "C-137", "Cronenberg_World", "Blips_and_Chitz", 
        "Anatomy_Park", "Planet_Squanch", "Greasy_Grandma_World", 
        "Interdimensional_Customs", "Citadel_of_Ricks", "Gromflom_Prime"
    };
    
    for(int i = 0; i < 7; i++){
        rms[i] = malloc(sizeof(Room));
        if(rms[i] == NULL){
            perror("Malloc failed when init'ing room\n");
        }

        rms[i]->id = i;
        rms[i]->name = roomNames[getName()];
        rms[i]->type = roomTypes[ (i > 1) ? 2 : i ];
        rms[i]->numConnections = 0;        
    }
}

// Randomly make connections among the rooms
void makeConnections(Room* rms[]){
    int numToMake, k;

    for(int i = 0; i < 7; i++){
        numToMake = (rand() % 4) + 3;
        while(rms[i]->numConnections < numToMake){
            k = rand() % 7;           
            if(isValidConnection(rms[i], rms[k])){
                connectRooms(rms[i], rms[k]);
            }
        } 
    }
}

bool isValidConnection(Room* a, Room* b){
    return  isDifferentRoom(a, b) && isNewNeighbor(a, b) && isAvailable(b);
}

// A room can't connect to itself.
bool isDifferentRoom(Room* a, Room* b){
    return a->id != b->id;
}

// A room can't connect to another room more than once
bool isNewNeighbor(Room* a, Room* b){
    for(int i = 0; i < a->numConnections; i++){
        if(a->connections[i] == b){
            return false;
        }
    }
    return true;
}

// A room must not exceed 6 connections
bool isAvailable(Room* b){
    return b->numConnections < 6;
}

// If room A connects to room B, then room B connects to room A
void connectRooms(Room* a, Room* b){
    a->connections[a->numConnections] = b;
    a->numConnections++;

    b->connections[b->numConnections] = a;
    b->numConnections++;
}

// Create a single room file using the room data (helper for createRoomFiles)
void _roomFile(Room* rms[], char* dirp, int i){
    FILE* fp;
    char fullp[50]; memset(fullp, '\0', 50);
    snprintf(fullp, 50, "%s/%s_room", dirp, rms[i]->name);
    
    fp = fopen(fullp, "w");
    if(fp == NULL){
        perror("Error opening room file for writing.\n");
    }
    
    fprintf(fp, "ROOM NAME: %s\n", rms[i]->name);
    for(int k = 0; k < rms[i]->numConnections; k++){
        fprintf(fp, "CONNECTION %d: %s\n", k + 1, rms[i]->connections[k]->name);
    }
    fprintf(fp, "ROOM TYPE: %s\n", rms[i]->type);
    fclose(fp);
}

void createRoomFiles(Room* rms[]){
    char dirPath[20]; memset(dirPath, '\0', 20);
    snprintf(dirPath, 20, "evvik.rooms.%d", getpid());
	mkdir(dirPath, 0755);   

    for(int i = 0; i < 7; i++){
        _roomFile(rms, dirPath, i);
    }
}// EOF
