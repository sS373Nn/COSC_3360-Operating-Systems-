TO COMPILE:

DO NOT NEED TO DO SO. ALREADY COMPILED.

However if you want to test it:

Part 1:

Each .cpp file needs to be compiled as it's own object for properly linking functions to programs.
This isn't as big of a deal for part 1, but since I originally built part 1 to be part of an overarching main function that simply output part1 and part2 to the console sequentially, this is how it works.

To compile each .cpp into an object call:

g++ -c functions.cpp -o functions.o -pthread -lrt
g++ -c main.cpp -o main.o -pthread -lrt

To link the objects call:

g++ main.o functions.o -o main -pthread -lrt

Part 2:

Each .cpp file needs to be compiled as it's own object for properly linking functions to programs.

To compile each .cpp into an object call:

g++ -c functions.cpp -o functions.o -pthread -lrt
g++ -c main.cpp -o main.o -pthread -lrt
g++ -c output_processor.cpp -o output_processor.o -pthread -lrt

To link the objects call:

g++ main.o functions.o -o main -pthread -lrt
g++ output_processor.o functions.o -o output_processor -pthread -lrt

There is probably a better, cleaner way to do this, but this worked and I'm not sure how else to do it.

////////////////////////////////////////////////////////////////////////

USAGE:

Navigate to:
Assignment1/src
(If you're reading this, you're in the folder Assignment1)

From here call:

cd part1

OR 

cd part2

Depending on if you want to navigate to part1 or part2.

Then call:

./main 

This will run the program for either part.

Enter the number of processes for the matrix when prompted.

Ex:

6

Enter the FULL matrix. Spaces between integers, no space after last integer in each line, INCLUDE '\n'. 

Ex:

0 0 0 0 1 0
0 0 0 0 0 1
0 0 0 1 0 0
0 0 0 0 1 1
0 0 0 0 0 0
0 0 0 0 0 0

The matrix entered will be printed to console with a prompt asking the user to accept or decline the matrix.
Accept with 'y' decline with 'n', though any input that isn't 'y' will be counted as an 'n'.

Program will output solution for part 1 to console.

In addition to this, Part 2 will prompt the user for text input.
You may either enter one word at a time or enter all words at once WITH '\n' between entries for the number of input processes in the matrix entered in part 1.

Ex:

apple
banana
grape

Program will output solution for part 2 to console.

////////////////////////////////////////////////////////////////////////

Functions:

For in depth function explanations please see functions.hpp file.
I've annotated function intent and methods to some depth for each function.
I've also saved many legacy functions that I did not use in the final code which can be passed over.
