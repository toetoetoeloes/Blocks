# Blocks
Blocks is a falling blocks game for Windows 10, written in C and the Windows API.

Blocks does not use sound. The function PlaySound() slows down the application after a while and I didn't want to use a different API/library.

New blocks do not spawn in two hidden top rows but immediately in the grid in the second and third top row.

The program creates a new file Blocks.dat in the same directory as the program itself to save the high scores.

To compile the program with MinGW, run GNU make. All files will be placed in the same directory. Make sure the environment variable LIBRARY_PATH is set or modify the makefile. 
