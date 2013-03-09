// WS1 - Glutpaint.cpp : Defines the entry point for the console application.

#include "stdafx.h"

// Mendefinisikan macam-macam objek
#define NULL        0
#define OBJECTLIST  0
#define LINE        1
#define RECTANGLE   2
#define TRIANGLE    3
#define POINTS      4
#define TEXT        5
#define POLYGON     6

// Untuk menyimpan objek apa saja yang ada
// untuk keperluan redraw
int   objtypes       [99999];  // Menyimpan jenis objek pada objek ke-n
int   objposoffset   [99999];  // Menyimpan posisi ke-i dari objek ke-n
int   nextoffset;
float objpositions   [99999];  // Menyimpan posisi objek untuk objek (tergabungkan)
int   objtypelength  [99999];
int   totalobj;
bool  objfilled      [99999];

// Object color
float objcolorr      [99999];
float objcolorg      [99999];
float objcolorb      [99999];

// Object text
// Digunakan untuk banyak objek
// Source: http://stackoverflow.com/questions/1088622/how-do-i-create-an-array-of-strings-in-c
typedef struct charsstring 
{
   char name[100]; // 100 character array
} charsstring;

charsstring objchar[99999];

// Cache
float positioncache [999];
int   totalcache;

void mouse(int, int, int, int);
void key(unsigned char, int, int);
void display(void);
void drawSquare(int, int);
void myReshape(GLsizei, GLsizei);

void myinit(void);

void screen_box(int, int, int);
void right_menu(int);
void middle_menu(int);
void color_menu(int);
void pixel_menu(int);
void fill_menu(int);
int pick(int, int);

/* globals */

GLsizei wh = 500, ww = 500; /* initial window size */
GLsizei ih = 500, iw = 500;
GLfloat size = 3.0;   /* half side length of square */
int draw_mode = 0; /* drawing mode */
int rx, ry; /*raster position*/

GLfloat r = 1.0, g = 1.0, b = 1.0; /* drawing color */
int fill = 0; /* fill flag */

void drawSquare(int x, int y)
{

    y=wh-y;
    glColor3ub( (char) rand()%256, (char) rand()%256, (char) rand()%256); 
    glBegin(GL_POLYGON);
    glVertex2f(x+size, y+size);
    glVertex2f(x-size, y+size);
    glVertex2f(x-size, y-size);
    glVertex2f(x+size, y-size);
    glEnd();
}


/* rehaping routine called whenever window is resized
or moved */

void myReshape(GLsizei w, GLsizei h)
{
    /* adjust clipping box */

    glViewport(0,0,w,h);
    double normalizedh = (double) 500.0 - (double) 0.5 * (double) h; //(double) h;
    double normalizedw = 0.0;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, w, -h/2.0 + normalizedh, h/2.0 + normalizedh, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 

    glClearColor (0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    display();
    glFlush();
    
    // berikan informasi panjang dan lebar saat ini
    // untuk dipergunakan fungsi lain
    ww = w;
    wh = h; 
}

// type: tipe objeknya. Length: berapa kali objmAddPosition dipanggil
void objmAddObject(int type, int length, float r, float g, float b) {
    objtypes     [totalobj] = type;
    objtypelength[totalobj] = length;
    objposoffset [totalobj] = nextoffset;
    objcolorr    [totalobj] = r;
    objcolorg    [totalobj] = g;
    objcolorb    [totalobj] = b;
    totalobj++;
}

void objmAddPosition(float x, float y) {
    objpositions[nextoffset++] = x;
    objpositions[nextoffset++] = y;
}


void myinit(void)
{
    glViewport(0,0,ww,wh);

    /* Pick 2D clipping window to match size of X window 
    This choice avoids having to scale object coordinates
    each time window is resized */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); 
    glOrtho(0.0, (GLdouble) ww , 0.0, (GLdouble) wh , -1.0, 1.0);

    /* set clear color to black and clear window */

    glClearColor (0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // Tes nambah objek
    totalobj = 0;
    nextoffset = 0;

    // Buat kotak
    objmAddObject(POLYGON, 4, 0.5, 0.7, 0.1);
    objmAddPosition(0.0, 0.0);
    objmAddPosition(0.0, 100.0);
    objmAddPosition(100.0, 100.0);
    objmAddPosition(75.0, 150.0);
    objmAddPosition(100.0, 0.0);

}

void drawObject() {
    int iteratedpos = 0;
    int baseindex = 0;
    for (int i = 0; i < totalobj; i++) {
        // Atur warnanya dulu
        glColor3f(objcolorr[i], objcolorg[i], objcolorb[i]);
        switch (objtypes[i]) {
            case LINE:
                baseindex = objposoffset[i];
                // Dengan cara ini, kita bisa copy paste programming
                glBegin(GL_LINES);
                glVertex2f((GLfloat) objpositions[baseindex], (GLfloat) objpositions[baseindex+1]);
                baseindex += 2;
                glVertex2f((GLfloat) objpositions[baseindex], (GLfloat) objpositions[baseindex+1]);
                glEnd();
                break;
            case RECTANGLE:
            case TRIANGLE:
            case POLYGON:
                glBegin(GL_POLYGON);
                for (int j = 0; j < objtypelength[i]; j++) {
                    baseindex = j * 2 + objposoffset[i];
                    glVertex2f((GLfloat) objpositions[baseindex], (GLfloat) objpositions[baseindex+1]);
                }
                glEnd();
                break;
            case TEXT:
                printf ("Unimplemented!");
                break;
            case POINTS:
                
                break;
        }
    }
    glFlush();
}

void mouse(int btn, int state, int x, int y)
{
    static int count;
    int where;
    static int xp[2],yp[2];
    if(btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN) 
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        where = pick(x,y);
        glColor3f(r, g, b);
        if(where != 0)
        {
            count = 0;
            draw_mode = where;
        }
        else switch(draw_mode) {
        case(LINE):
            if(count==0) {
                count++;
                xp[0] = x;
                yp[0] = y;
            } else {
                objmAddObject(LINE, 2, 0.3, 0.3, 0.3);
                objmAddPosition(xp[0], wh-yp[0]);
                objmAddPosition(x    , wh-y);
                
                count=0;
            }
            break;
        case(RECTANGLE):
            if(count == 0) {
                count++;
                xp[0] = x;
                yp[0] = y;
            } else {
                objmAddObject(POLYGON, 4, 0.2, 0.2, 0.8);
                objmAddPosition(x, wh-y);
                objmAddPosition(x, wh-yp[0]);
                objmAddPosition(xp[0], wh-yp[0]);
                objmAddPosition(xp[0], wh-y);
                count=0;
            }
            break;
        case (TRIANGLE):
            switch(count)
            {
            case(0):
                count++;
                xp[0] = x;
                yp[0] = y;
                break;
            case(1):
                count++;
                xp[1] = x;
                yp[1] = y;
                break;
            case(2):
                objmAddObject(POLYGON, 3, 0.8, 0.2, 0.8);
                objmAddPosition(xp[0],wh-yp[0]);
                objmAddPosition(xp[1],wh-yp[1]);
                objmAddPosition(x,wh-y);
                count=0;
            }
            break;
        case(POINTS):
            {
                objmAddObject(POINTS, 1, 0.2, 0.2, 0.2);
                objmAddPosition(x,y);
                count++;
            }
            break;
        case(TEXT): 
            {
                rx=x;
                ry=wh-y;
                glRasterPos2i(rx,ry); 
                count=0;
            }
        }

        glPopAttrib();
        glFlush();
    }

    drawObject();
}

int pick(int x, int y)
{
    y = wh - y;
    if(y < wh-ww/10) return 0;
    else if(x < ww/10) return LINE;
    else if(x < ww/5) return RECTANGLE;
    else if(x < 3*ww/10) return TRIANGLE;
    else if(x < 2*ww/5) return POINTS;
	else if(x < ww/2) return TEXT;
    else return 0;
}

void screen_box(int x, int y, int s )
{
    glBegin(GL_QUADS);
      glVertex2i(x, y);
      glVertex2i(x+s, y);
      glVertex2i(x+s, y+s);
      glVertex2i(x, y+s);
    glEnd();
}

void right_menu(int id)
{
   if(id == 1) exit(0);
   else display();
}

void middle_menu(int id)
{

}

void color_menu(int id)
{
   if(id == 1) {r = 1.0; g = 0.0; b = 0.0;}
   else if(id == 2) {r = 0.0; g = 1.0; b = 0.0;}
   else if(id == 3) {r = 0.0; g = 0.0; b = 1.0;}
   else if(id == 4) {r = 0.0; g = 1.0; b = 1.0;}
   else if(id == 5) {r = 1.0; g = 0.0; b = 1.0;}
   else if(id == 6) {r = 1.0; g = 1.0; b = 0.0;}
   else if(id == 7) {r = 1.0; g = 1.0; b = 1.0;}
   else if(id == 8) {r = 0.0; g = 0.0; b = 0.0;}
}


void pixel_menu(int id)
{
   if (id == 1) size = 2 * size;
   else if (size > 1) size = size/2;
}

void fill_menu(int id)
{
   if (id == 1) fill = 1; 
   else fill = 0;
}

void key(unsigned char k, int xx, int yy)
{
   if(draw_mode!=TEXT) return;
	glColor3f(0.0,0.0,0.0);
   glRasterPos2i(rx,ry);
   glutBitmapCharacter(GLUT_BITMAP_9_BY_15, k);
	/*glutStrokeCharacter(GLUT_STROKE_ROMAN,i); */
   rx+=glutBitmapWidth(GLUT_BITMAP_9_BY_15,k);

}



void display(void)
{
	int shift=0;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClearColor (0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    screen_box(0,ih-iw/10,iw/10);
    glColor3f(1.0, 0.0, 0.0);
    screen_box(iw/10,ih-iw/10,iw/10);
    glColor3f(0.0, 1.0, 0.0);
    screen_box(iw/5,ih-iw/10,iw/10);
    glColor3f(0.0, 0.0, 1.0);
    screen_box(3*iw/10,ih-iw/10,iw/10);
    glColor3f(1.0, 1.0, 0.0);
    screen_box(2*iw/5,ih-iw/10,iw/10);
    glColor3f(0.0, 0.0, 0.0);
	screen_box(iw/10+iw/40,ih-iw/10+iw/40,iw/20);
    glBegin(GL_LINES);
       glVertex2i(ih/40,ih-iw/20);
       glVertex2i(ih/40+iw/20,ih-iw/20);
    glEnd();
    glBegin(GL_TRIANGLES);
       glVertex2i(iw/5+iw/40,ih-iw/10+iw/40);
       glVertex2i(iw/5+iw/20,ih-iw/40);
       glVertex2i(iw/5+3*iw/40,ih-iw/10+iw/40);
    glEnd();
    glPointSize(3.0);
    glBegin(GL_POINTS);
       glVertex2i(3*iw/10+iw/20, ih-iw/20);
    glEnd();
	glRasterPos2i(2*iw/5,ih-iw/20);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'A');
	shift=glutBitmapWidth(GLUT_BITMAP_9_BY_15, 'A');
	glRasterPos2i(2*iw/5+shift,ih-iw/20);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'B');
	shift+=glutBitmapWidth(GLUT_BITMAP_9_BY_15, 'B');
	glRasterPos2i(2*iw/5+shift,ih-iw/20);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'C');

    drawObject();

    glFlush();
    glPopAttrib();

    
}

int main(int argc, char** argv)
{
    int c_menu, p_menu, f_menu;

    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
    glutCreateWindow("FidPaint - Powered by GLUT | Worksheet 1 Computer Graphic");
    glutDisplayFunc(display);
    c_menu = glutCreateMenu(color_menu);
    glutAddMenuEntry("Red",1);
    glutAddMenuEntry("Green",2);
    glutAddMenuEntry("Blue",3);
    glutAddMenuEntry("Cyan",4);
    glutAddMenuEntry("Magenta",5);
    glutAddMenuEntry("Yellow",6);
    glutAddMenuEntry("White",7);
    glutAddMenuEntry("Black",8);
    p_menu = glutCreateMenu(pixel_menu);
    glutAddMenuEntry("increase pixel size", 1);
    glutAddMenuEntry("decrease pixel size", 2);
    f_menu = glutCreateMenu(fill_menu);
    glutAddMenuEntry("fill on", 1);
    glutAddMenuEntry("fill off", 2);
    glutCreateMenu(right_menu);
    glutAddMenuEntry("quit",1);
    glutAddMenuEntry("clear",2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutCreateMenu(middle_menu);
    glutAddSubMenu("Colors", c_menu);
    glutAddSubMenu("Pixel Size", p_menu);
    glutAddSubMenu("Fill", f_menu);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
    myinit ();
    glutReshapeFunc (myReshape); 
	glutKeyboardFunc(key);
    glutMouseFunc (mouse);
    glutMainLoop();

}


	