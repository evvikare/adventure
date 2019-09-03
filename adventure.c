/*******************************************************************************
*
* adventure.c
*
* -----------------
* EvVikare
* evvikare@protonmail.com
* CS344 -- Summer 2017
* Program 2 Assignment
* -----------------
*
* Guide player through rooms created with buildroom.c
*
*******************************************************************************/

#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define NUMROOMS 7
#define MAXPATH 512
#define MAXINPUT 1024

pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Room{
	int id;
	char* name;
	char* type;
	int numConnections;
	struct Room* connections[6];	
}Room;

typedef void(*room_func)(Room* [], FILE*, int);

char* getNewestDir();
void setRoomDataFromFile(Room* rms[], char* dName, room_func rf);
void setRoom(Room* rms[], FILE* fh, int id);
void setConnections(Room* rms[], FILE* fh, int id);
Room* getStartRoom(Room* rms[]);
void* writeTime(void* arg);
void readTime(FILE *fh);
int lookupByName(Room* rms[], char* query);
void prompt(Room* currentRoom);
Room* getPlayerInput(Room* rms[], Room* currRoom);
bool isGameOver(Room* rms[], Room* destination, Room* currRoom);
bool isConnected(Room* self, char* playerInput);
void errorMsg();
bool endRoomFound(Room* currentRoom);
void endMsg(Room* rms[], int history[], int numSteps);
void cleanUp(Room* rms[], char* dirname);

int main(){

    Room* rooms[NUMROOMS];
    Room *currRoom, *destination;

    char* advDir = getNewestDir();
    setRoomDataFromFile(rooms, advDir, setRoom);
    setRoomDataFromFile(rooms, advDir, setConnections);
    currRoom = getStartRoom(rooms);
    destination = currRoom;
    
    while( ! isGameOver(rooms, destination, currRoom) ){
        currRoom = destination;
        prompt(currRoom);
        destination = getPlayerInput(rooms, currRoom);
    }

    cleanUp(rooms, advDir);

    return 0;
}

// Returns newest directory with Adventure rooms in it
char* getNewestDir(){
    DIR* currentDir;
    struct dirent *currentFile;
    struct stat attributes;
    int mostRecent = 0;
    char* newestDir = malloc(50);

    if(currentDir = opendir(".")){
        while(currentFile = readdir(currentDir)){
            if(strstr(currentFile->d_name, "evvik.rooms.")){       
                stat(currentFile->d_name, &attributes);
                
                if((int) attributes.st_mtime > mostRecent){
                    mostRecent = (int) attributes.st_mtime;
                    memset(newestDir, '\0', 50);
                    strcpy(newestDir, currentFile->d_name);
                }
            }
            currentFile = readdir(currentDir);
        }
    }
        
    closedir(currentDir);  
    return newestDir;
}

// Return file handle for given path and mode
FILE* getFileHandle(char* fullPath, char* fileMode){
    FILE* fh = fopen(fullPath, fileMode);
    if(fh == NULL){
        perror("Error opening file.\n");
    }
    return fh;
}

// General purpose function for setting room fields and connections
void setRoomDataFromFile(Room* rms[], char* dName, room_func rf){
    struct dirent *currFile;
    char fullPath[MAXPATH];
    DIR* rdir = opendir(dName);
    FILE* fh;
    int i = 0;
    
    while(i < NUMROOMS){
        currFile = readdir(rdir);
        if(currFile && (strlen(currFile->d_name) > 2)){ // Skip . and ..
            // Load new path into buffer
            memset(fullPath, '\0', MAXPATH);
            snprintf(fullPath, MAXPATH, "%s/%s", dName, currFile->d_name);
            
            // Open file and set data using room function
            fh = getFileHandle(fullPath, "r");
            rf(rms, fh, i);
            fclose(fh);
            
            i++;
        }
    }
    closedir(rdir);
}

// Write to time file and read to console
void updateTime(){
    FILE* fh = getFileHandle("./currentTime.txt", "w");
    pthread_t write_time;
    pthread_mutex_lock(&time_mutex);
    pthread_create(&write_time, NULL, writeTime, fh);
    pthread_mutex_unlock(&time_mutex);            
    pthread_join(write_time, NULL);
    fclose(fh);

    FILE* fh_read = getFileHandle("./currentTime.txt", "r");
    readTime(fh_read);   
    fclose(fh_read);
}

// Set Room struct fields
void setRoom(Room* rms[], FILE* fh, int id){
    char col3[25];
    int i;
    
    rms[id] = malloc(sizeof(Room));
    char* rm_name = (char*) malloc(25);
    char* rm_type = (char*) malloc(25);    
    if(rms[id] == NULL || rm_name == NULL || rm_type == NULL){
        perror("Malloc failed when fetch'ing room\n");
    }
    
    for(i = 0; fscanf(fh, "%*s %*s %s", col3) != EOF; i++){  
        if(i == 0){
            strcpy(rm_name, col3); // First value of col3 is the room name   
        }
    }

    strcpy(rm_type, col3); // The last value of col3 is the room type
    
    rms[id]->id = id;
    rms[id]->name = rm_name;
    rms[id]->type = rm_type;
    rms[id]->numConnections = i - 2;
}

//Set Room struct connections array
void setConnections(Room* rms[], FILE* fh, int id){
    int k = 0;
    char col1[15], col3[25];

    for(k = 0; fscanf(fh, "%s %*s %s", col1, col3) != EOF; k++){
       if(strstr(col1, "CONNECTION")){
            rms[id]->connections[k - 1] = rms[lookupByName(rms, col3)];
        }
    }
}

// Return game's starting room
Room* getStartRoom(Room* rms[]){
    int i;
    for(i = 0; i < NUMROOMS; i++){
        if(strcmp(rms[i]->type, "START_ROOM") == 0){
            return rms[i];
        }
    }
}

// Writes time to currentTime file
void* writeTime(void* arg){
    FILE* fh = arg;
    char outputBuffer[50];    
    time_t rawTime = time(0);
    struct tm* currentTime = localtime(&rawTime);

	strftime(outputBuffer, 50, "%l:%M%P, %A, %B %e, %Y", currentTime); 
    fprintf(fh, "%s\n", outputBuffer);
    
    return NULL;
}

// Read time from currentTime file
void readTime(FILE *fh){
    char inputBuffer[50];
    fgets(inputBuffer, 50, fh);
    printf("%s\n", inputBuffer);
}

// Print the game prompt
void prompt(Room* currentRoom){
    printf("\nCURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS:", currentRoom->name);
    for(int i = 0; i < currentRoom->numConnections; i++){
        printf(" %s", currentRoom->connections[i]->name);
        printf("%c", (i < currentRoom->numConnections - 1) ? ',' : '.');
    }
    printf("\nWHERE TO? >");
}

// Return the player's valid input room, defaults to current room
Room* getPlayerInput(Room* rms[], Room* currRoom){
    Room* destination = currRoom; // dest is currRoom by default (for time)
    char playerInput[MAXINPUT]; memset(playerInput, '\0', MAXINPUT);
    int roomIdx;

    fgets(playerInput, MAXINPUT, stdin);
    playerInput[strlen(playerInput) - 1] = '\0';
    roomIdx = lookupByName(rms, playerInput);

    if(strcmp(playerInput, "time") == 0){
        printf("\n");
        updateTime();
    }else if((roomIdx >= 0) && isConnected(currRoom, playerInput)){
        destination = rms[roomIdx];
    }else{
        errorMsg();
    }
    
    return destination;
}

// Returns true if game is over
bool isGameOver(Room* rms[], Room* destination, Room* currRoom){
    static int numSteps = 0;        
    static int history[64];
    bool atEnd = endRoomFound(destination);

    // Only update history if valid move
    if(destination != currRoom){
        history[numSteps] = destination->id;
        numSteps++;
    }

    // Print winning message if end room found
    if(atEnd){
        endMsg(rms, history, numSteps);
    }

    return atEnd;
}

// Return a room's index in the rooms array, or -1 if not found
int lookupByName(Room* rms[], char* query){
    for(int i = 0; i < NUMROOMS; i++){
        if(strcmp(rms[i]->name, query) == 0){
            return i;        
        }
    }
    return -1;
}

// Return true if self and the player's input are connected
bool isConnected(Room* self, char* playerInput){
    int i;
    for(i = 0; i < self->numConnections; i++){
        if(strcmp(self->connections[i]->name, playerInput) == 0){
            return true;
        }
    }
    return false;
}

// Print if non-valid text is entered by the user
void errorMsg(){
    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
}

// Return true if the player's move is to the end room
bool endRoomFound(Room* currentRoom){
    return strcmp(currentRoom->type, "END_ROOM") == 0;
}

// Print a game-over message, including the player's step history
void endMsg(Room* rms[], int history[], int numSteps){	
	printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", numSteps);

    for(int i = 0; i < numSteps; i++){
        printf("%s\n", rms[history[i]]->name);
    }  
}

// Free malloc'ed memory
void cleanUp(Room* rms[], char* dirname){
    for(int i = 0; i < NUMROOMS; i++){
        free(rms[i]->name);
        free(rms[i]->type);
        free(rms[i]);
    }

    free(dirname);
}//EOF


