**IMPORTANT**

BE SURE TO REMOVE ANY EXTRA CHARACTERS FROM THE .txt INPUT FILE IF YOU CREATED IT IN WINDOWS OR THIS PROGRAM WILL NOT WORK.

Run:
sed -i 's/\r//g' [filename].txt

This will remove the extra bs Windows adds to the .txt files for newline.
If you don't, my program will not run properly as it is designed to parse linux .txt files.

**END IMPORTANT**

NOTE:

This is not as important as it was for assignment2, but it won't hurt.

////////////////////////////////////////////////////////////////////////

TO COMPILE:

DO NOT NEED TO DO SO. ALREADY COMPILED.

However if you want to test it:

As with assignment1 and assignment2, each .cpp file needs to be compiled as it's own object for properly linking functions to programs.

To compile each .cpp into an object call:

g++ -c functions.cpp -o functions.o
g++ -c main.cpp -o main.o

To link the objects call:

g++ main.o functions.o -o main -pthread -lrt

Still haven't found a better way, but if it aint broke...

////////////////////////////////////////////////////////////////////////

USAGE:

Navigate to:
assignment3/src
(If you're reading this, you're in the folder assignment2)

From here call:

cd src

To run call:

./main m x < [filename].txt

Where m == memory_size and x == look_ahead

***FILE INPUT DONE WITH STD::CIN***

The page input file is fed into the program using std::cin and should be used as shown above.

Input file example:

1:a 1:a 1:n 1:a 2:n 3:a 3:n 3:n 4:a 4:n 4:a\n


Program will then output solutions to console as follows:

"Current [Page Replacement Type] Set:\n"
"[[Memory_Contents]]\n"
"Number of Replacements: [page_replacements]\n"
"Number of Misses: []\n"
"Number of Delayed Replacements: []\n"
"Page References:\n"
"[page]: [reference_count]\n"
"[page]: [reference_count]\n"
...
"Total References: [page_reference_sum]\n\n"

NOTE:

This program was built assuming look_ahead (x value input) is never >= input data size (number of pages in the input file).
If it is, this program will not properly run OPT or OPT_MOBILE.

////////////////////////////////////////////////////////////////////////

Functions:

For in depth function explanations please see functions.hpp file.
I've annotated function intent and methods to some depth for each function.
While in the past, I saved many of my legacy functions that I replace or no longer use, in this project they were causing compiler issues with some of my last updates and it made more sense for me to just remove most of them.
I've cleaned up all of my notes and annotations from my functions.cpp file for ease of reading.
Upon request I can supply a version of functions.cpp that has been unmodified at completion, leaving all my original notes and explanations in.
As of now, I've removed all of those and left the explanations to .hpp as it seems easier to read and parse which function is which like that.

Though not required, I chose to use fork()s here in order to run all of the different page replacement programs simultaneously.
I find that it actually simplified the logic a bit in some regards as it allows me to simply feed each token to each process as it's read and not store it in a vector, though it did make other things a bit more difficult, especially syncing the OPT and OPT-MOBILE methods.
Once everything was said and done, I think it was still worth it.

Overall I'm happy with the logic of my program, but there is MUCH to be desired in terms of efficiency and DRY.
Given more time I'd like to rework many of my functions that use similar logic or have a difference of looking for 'a' vs 'n' and improving them, but for now I've got too many exams to study for.

////////////////////////////////////////////////////////////////////////

Bonus:

For my bonus OPT-MOBILE I built an algorithm that prioritizes replacing pages with bandwidth available in the future over pages that will not have bandwidth in the future.
On a miss, the algorithm prioritizes finding:
    1) the furthest first occurrence of a page in the future that has bandwidth 'a' that is not preceded by an occurrence of that page without bandwidth 'n'
    2) the furthest first occurrence of a page that will be needed in the future regardless of 'n' or 'a'
    3) any page from memory if neither of those criteria can be met

On a hit, the algorithm checks future_tokens to see if any not currently in memory will be needed when bandwidth is not available.
If so, the algorithm prioritizes finding:
    1) any pages in memory that will not be needed in the foreseeable future
    2) the furthest first occurrence of a page with bandwidth 'a' that is not preceeded by the same page with bandwidth 'n'
    
If neither of those criteria can be met, the algorithm does not discard any pages from memory and proceeds as normal.

This allows the algorithm to use the time that would normally be unused for page replacement by finding pages that will not be available on demand, thus minimizing occurrences of delayed replacements.
