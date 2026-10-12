# Hyper-GOL
Conway's Game of Life for Ubuntu

An implementation of Conway's Game of Life for Linux/Ubuntu. This version uses a 100 x 100 cell grid. It requires to have X11 installed. If the executable (for Ubuntu 24.04) does not work, you can recompile the source code by:

gcc gol.c -o gol -lm -lX11

You can stop the running program by clicking on its window then typing any key. 

gol2.c is the source of another version in which the grid "wraps around": opposite edges of the grid are touching each other (as if the grid was the surface of a torus). This usually leads to a longer evolution, before the cells get stuck in still shapes or are in oscillating states forever.
