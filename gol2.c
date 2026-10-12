// Classic Conway's Game of Life 

// Case of a wrap-around map

// By S. Morel, Zthorus-Labs, 2026

/* Re-used the X-Window stuff from pengo.c
*/

/* Change-Log:
   -----------
*/

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Map constants

#define X_MAP 100  // Map size (including borders)
#define Y_MAP 100


// Codes of keys (similar to keysymdef.h)

#define LEFT_ARROW_K  0xFF51 // XK_Left
#define RIGHT_ARROW_K 0xFF53 // XK_Right
#define UP_ARROW_K    0xFF52 // XK_Left
#define DOWN_ARROW_K  0xFF54 // XK_Right
#define QUIT_K 0x0051        // Doesn't seem to work
#define PAUSE_K 0x0050
#define ESC_K 0xFF1B

void CreateWalker(int **map, int x, int y, int dir);

void CreateFlower(int **map, int x, int y);

void CreateColor(Display *display,Colormap colmap,unsigned long *lut,int i,int r,int g,int b);
 
void Dot(XImage *xim,unsigned long *lut,int x,int y,int color);
 
long GetTheKeyNoBlock(Display *display);

void WaitKeyReleased(Display *display);


int main(int argc, char **argv)
{
  // X11-specific variables

  Display *display;
  Visual  *visu;
  XEvent report;
  int screen;
  unsigned int depth;
  GC gc;
  Window win;
  XSizeHints sizeHints;
  char *im;
  XImage *xim;       // Pixmap displayed in the window
  Colormap colmap;
  unsigned long lut[16];  // Look-up Table of colors

  int **map;          // Map of the game
  int **next_map;     // Map at next iteration

  int xw = 3*X_MAP;
  int yw = 3*Y_MAP;

  int i,j,k,v,e;
  int running;
  int key;

 
   // Memory allocations
 
  map = (int **)malloc(X_MAP*sizeof(int *));
  if (map == NULL)
  {
    printf("Cannot allocate memory for array: map\n");
    exit(0);
  }
  for (i = 0 ; i < X_MAP ; i++)
  {
    map[i] = (int *)malloc(Y_MAP*sizeof(int));
    if (map[i] == NULL)
    {
      printf("Cannot allocate memory for array: map[%d]\n",i);
      exit(0);
    }
  }

  next_map = (int **)malloc(X_MAP*sizeof(int *));
  if (next_map == NULL)
  {
    printf("Cannot allocate memory for array: next_map\n");
    exit(0);
  }
  for (i = 0 ; i < X_MAP ; i++)
  {
    next_map[i] = (int *)malloc(Y_MAP*sizeof(int));
    if (next_map[i] == NULL)
    {
      printf("Cannot allocate memory for array: next_map[%d]\n",i);
      exit(0);
    }
  }


  // Create the display 

  display = XOpenDisplay(NULL);
  if (display == NULL)
  {
    fprintf(stderr,"Cannot connect to X-server\n");
    exit(-1);
  }
  screen = DefaultScreen(display);
  visu = XDefaultVisual(display,screen);
  gc = XCreateGC(display,RootWindow(display,screen),0,NULL);
  depth = DisplayPlanes(display,screen);

  // Create the window 
     
  win = XCreateSimpleWindow(display,RootWindow(display,screen),100,100,xw,yw,4,BlackPixel(display,screen),WhitePixel(display,screen));
  sizeHints.flags = PPosition|PSize;
  sizeHints.x = 100;
  sizeHints.y = 100;
  sizeHints.width = xw;
  sizeHints.height= yw;
  XSetStandardProperties(display,win,"GOL","GOL",0,argv,argc,&sizeHints);
  XSelectInput(display,win,ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|StructureNotifyMask);

  // Create the color map

  colmap=XCreateColormap(display,win,visu,AllocNone);
  XSetWindowColormap(display,win,colmap);
  XInstallColormap(display,colmap);
  XMapWindow(display,win);

  CreateColor(display,colmap,lut,0,0,0,0);
  CreateColor(display,colmap,lut,1,0,60000,60000);
 
  // Create the pixmap of the window 

  im=(char *)malloc(4*xw*yw*sizeof(char));
  if (im==NULL)
  {
    fprintf(stderr,"Cannot allocate memory for im (window pixmap)\n");
    exit(-1);
  }
  xim=XCreateImage(display,visu,depth,ZPixmap,0,im,xw,yw,32,0);


  // Reset maps

  for (i = 0; i < X_MAP ; i++)
  {
    for (j = 0; j < Y_MAP ; j++)
    {
      map[i][j] = 0;
      next_map[i][j] = 0;
    }
  }

  // Create initial pattern
 
/* 
  for (i = 30; i < 74 ; i++)
  {
    for (j = 30; j < 71 ; j++)
    {
      k = rand()%3;
      if (k == 0) map[i][j] = 1; 
    }
  }
*/


  for (i = 0 ; i < 100 ; i++)
  {
    CreateWalker(map,rand()%90+5,rand()%90+5,i%4);
  }

  CreateFlower(map,50,50);

  running = 1;

  // Main loop

  while(running) 
  {
    // Display map

    for (i = 0; i < X_MAP ; i++)
    {
      for (j = 0; j < Y_MAP ; j++)
      {
        Dot(xim,lut,i,j,map[i][j]);
      }
    }

    XPutImage(display,win,gc,xim,0,0,0,0,xw,yw);
    //usleep(1000000);
    usleep(100000);
    
    // Create next iteration according to GOL's rules
  
    e = 0;
 
    for (i = 0 ; i < X_MAP ; i++)
    {
      for (j = 0 ; j < Y_MAP ; j++)
      {
        v = map[(i-1+X_MAP)%X_MAP][(j-1+Y_MAP)%Y_MAP]
            + map[i][(j-1+Y_MAP)%Y_MAP] + map[(i+1)%X_MAP][(j-1+Y_MAP)%Y_MAP]
            + map[(i-1+X_MAP)%X_MAP][j] + map[(i+1)%X_MAP][j]
            + map[(i-1+X_MAP)%X_MAP][(j+1)%Y_MAP] + map[i][(j+1)%Y_MAP]
            + map[(i+1)%X_MAP][(j+1)%Y_MAP];
        // cell creation
        if ((map[i][j] == 0) && (v == 3))
        {
          next_map[i][j] = 1;
          e++;
        }
        else next_map[i][j] = 0;
        // cell survival or death
        if (map[i][j] == 1)
        {
          if ((v != 2) && (v != 3))
          { 
            next_map[i][j] = 0;
            e++;
          }
          else next_map[i][j] = 1;
        }
      }
    }
    //printf("Energy = %d\n", e);

    for (i = 0; i < X_MAP ; i++)
    {
      for (j = 0; j < Y_MAP ; j++)
      {
        map[i][j] = next_map[i][j];
      }
    }

    key = GetTheKeyNoBlock(display);
    if (key != -1) running = 0;
  }
 
  exit(0);
 
}


// Create a "walker" on the map

void CreateWalker(int **map, int x, int y, int dir)
{
  switch(dir)
  {
    case 0:
      map[x+1][y] = 1;
      map[x+2][y+1] = 1;
      map[x][y+2] = 1;
      map[x+1][y+2] = 1;
      map[x+2][y+2] = 1; 
      break;
    case 1:
      map[x+1][y] = 1;
      map[x][y+1] = 1;
      map[x][y+2] = 1;
      map[x+1][y+2] = 1;
      map[x+2][y+2] = 1; 
      break;
    case 2:
      map[x][y] = 1;
      map[x+1][y] = 1;
      map[x+2][y] = 1;
      map[x+2][y+1] = 1;
      map[x+1][y+2] = 1; 
      break;
    default:
      map[x][y] = 1;
      map[x+1][y] = 1;
      map[x+2][y] = 1;
      map[x][y+1] = 1;
      map[x+1][y+2] = 1; 
      break;
  }
}

// Create a blooming flower on the map

void CreateFlower(int **map, int x, int y)
{
  map[x-1][y-1] = 1;
  map[x][y-1] = 1;
  map[x+1][y-1] = 1;
  map[x-2][y] = 1;
  map[x][y] = 1;
  map[x+2][y] = 1;
  map[x-1][y+1] = 1;
  map[x][y+1] = 1;
  map[x+1][y+1] = 1;
}

// Function to create a color in the LUT 

void CreateColor(Display *display,Colormap colmap,unsigned long *lut,int i,int r,int g,int b)
{
  XColor colorx;

  colorx.red = r;
  colorx.green = g;
  colorx.blue = b;
  XAllocColor(display,colmap,&colorx);
  lut[i] = colorx.pixel;
}




   
// Function to draw a dot (that is actually a block of 3x3 pixels) 

void Dot(XImage *xim,unsigned long *lut,int x,int y,int color)
{ 
  int xx,yy;
  
  xx = 3*x;
  yy = 3*y;

  XPutPixel(xim,xx,yy,lut[color]);
  XPutPixel(xim,xx+1,yy,lut[color]);
  XPutPixel(xim,xx+2,yy,lut[color]);
  XPutPixel(xim,xx,yy+1,lut[color]);
  XPutPixel(xim,xx+1,yy+1,lut[color]);
  XPutPixel(xim,xx+2,yy+1,lut[color]);
  XPutPixel(xim,xx,yy+2,lut[color]);
  XPutPixel(xim,xx+1,yy+2,lut[color]);
  XPutPixel(xim,xx+2,yy+2,lut[color]);
} 


// Function to poll the keyboard

long GetTheKeyNoBlock(Display *display)
{
  XEvent report;
  char kBuf[2];
  long kk;
  KeySym theKey;
  int stillPressed;
  
  kk=-1;
  
  if (XCheckMaskEvent(display,KeyPressMask,&report) == True)
  { 
    XLookupString(&(report.xkey),kBuf,2,&theKey,NULL);
    kk=(long)theKey;
    stillPressed = 1;
    while(stillPressed)
    {  
      if (XCheckMaskEvent(display,KeyPressMask,&report) == True)
      {
        stillPressed = 1;
      }
      else stillPressed = 0;
    }
  }
  return(kk);
}


// Function to make sure no key is pressed

void WaitKeyReleased(Display *display)
{
  long done;
  XEvent report;

  done = 0; 
  while (!done)
  { 
    XNextEvent(display,&report);
    if (report.type == KeyRelease) done = 1;
  }
}



