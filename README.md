# adventure

adventure is a C program that approximates text-based RPGs like its namesake.
This was an assignment completed for CS344 Operating Systems at OSU.

## Usage

To compile the room-building program:

    gcc –o buildrooms buildrooms.c

To compile the adventure program:

    gcc –o adventure adventure.c -lpthread

To play, first build the directory that holds the room files:

    ./buildrooms

Then, run the adventure program:

    ./adventure

## Notes

Commands are case-sensitive and must be entered exactly as they appear.

The buildrooms program can be run multiple times to get a new, random maze.

The adventure program uses the most recently created rooms directory.

The rooms directories must be manually deleted.

To get the current time, enter "time" at the prompt instead of a room name.
