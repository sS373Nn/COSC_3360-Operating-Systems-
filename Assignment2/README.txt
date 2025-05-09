**IMPORTANT**

BE SURE TO REMOVE ANY EXTRA CHARACTERS FROM THE .txt INPUT FILE IF YOU CREATED IT IN WINDOWS OR THIS PROGRAM WILL NOT WORK.

Run:
sed -i 's/\r//g' [filename].txt

This will remove the extra bs Windows adds to the .txt files for newline.
If you don't, my program will not run properly as it is designed to parse linux .txt files.

**END IMPORTANT**

////////////////////////////////////////////////////////////////////////

TO COMPILE:

DO NOT NEED TO DO SO. ALREADY COMPILED.

However if you want to test it:

As with assignment1, each .cpp file needs to be compiled as it's own object for properly linking functions to programs.

To compile each .cpp into an object call:

g++ -c functions.cpp -o functions.o
g++ -c main.cpp -o main.o
g++ -c main_LLF.cpp -o main_LLF.o

To link the objects call:

g++ main.o functions.o -o main -pthread -lrt
g++ main_LLF.o functions.o -o main_LLF -pthread -lrt

Still haven't found a better way, but if it aint broke...

////////////////////////////////////////////////////////////////////////

USAGE:

Navigate to:
assignment2/src
(If you're reading this, you're in the folder assignment2)

From here call:

cd src

To run call:

./main <filename>.txt <filename>.txt

***FILE ORDER IS IMPORTANT***

First file MUST be the matrix and process data, second file the resource data.
For the next assignment I can probably write some code to put each where they need to be regardless of order, but this assigment took much longer than expected, so please order the input as mentioned.

Program will then output solutions to console, EDF first then LLF.

////////////////////////////////////////////////////////////////////////

Functions:

For in depth function explanations please see functions.hpp file.
I've annotated function intent and methods to some depth for each function.
I've also saved many legacy functions that I did not use in the final code which can be passed over.
This assignment was a bit chaotic compared to the first as I refactored most of my code MANY times as I gained a better understanding of what needed to be done and how organization should work out.
Upon request I can supply a version of functions.cpp that has been unmodified at completion, leaving all my original notes and explanations in.
As of now, I've removed all of those and left the explanations to .hpp as it seems easier to read and parse which function is which like that.
I've also included a test_functions.cpp that I used to test helper functions locally in Windows.
SOME of the tests in my test_functions.cpp file were written by ChatGPT to save time, but NONE of the function code itself nor any of the rest of my assignment is done using ChatGPT code.

NOTE:
This evening around 6 PM while checking over to be SURE my program was squared away I realized I had forgotten a very important part, printing the system state at every request allocation.
I've added all functions involved near the bottom of both my .hpp and .cpp
As it's 9 PM and I haven't eaten dinner yet, the annotations on them may a bit sloppy.
Please reach out if you need me to explain them better.
Fitting them into the program was also.... ugly.
So some of my initial function inputs that also now house my new functions will not be fully annotated.
Sorry for the sloppy work, again please reach out if I need to explain any of them in any more depth.

Thank you!
