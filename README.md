# Conway's Game of Life
The program performs a simulation of the famous Game of Life and responds to user input:
1. Input starting generation of living cells:
To input living cells in the first generation, simply follow the below syntax:
/{row} {column} {column} ... {column}
.
.
.
/{row} {column} ... {column}
/
The list of rows and columns must not repeat and be listed in ascending order, e.g.
/10 32 33 34 37 38 39 42 43 44 47 48 49
/11 34 37 39 44 49
/12 32 33 34 37 39 42 43 44 47 48 49
/13 32 37 39 42 47
/14 32 33 34 37 38 39 42 43 44 47 48 49
/
2. Calculate the next generation:
To calculate the next generation, press the ENTER key.
3. Calculate 'n' next generations:
To calculate the next 'n' generations, input the number 'n' and press ENTER.
4. Output the current positions of all living cells:
To output the current positions of all living cells in the same format as above, input '0' and press ENTER.
5. Shift the view:
To shift the current view, input the coordinates of a cell that should be in the upper-left corner of the window. The default upper-left corner is in row '1' and column '1'. For example, inputting '-3 5' will move the view so that the cell in row '-3' and column '5' will be in the upper-left corner.
6. Exit the simulation:
To exit the simulation, input a period '.'.

All user input except for the exit command is followed by outputting the current view. The living cells are represented as '0' and the dead cells are represented as '.'. The board is dynamically extended in all four directions whenever necessary.
