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
#define EDITMODE    7

// Mendefinisikan warnanya
#define COLOR1      7
#define COLOR2      8
#define COLOR3      9
#define COLOR4      10
#define COLOR5      11
#define COLORNOFILL 12

float  ccolorr; // Current color R
float  ccolorg; // Current color G
float  ccolorb; // Current color B
bool   fill;
int    activecolorenum;
int    activetoolenum;

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
void display(GLenum);
void drawSquare(int, int);
void myReshape(GLsizei, GLsizei);
void setcolor2(int, bool);
void myinit(void);

void screen_box(int, int, int);
void right_menu(int);
void middle_menu(int);
void color_menu(int);
void pixel_menu(int);
void fill_menu(int);
void setcolor(int);
int pick(GLint, GLuint*);
int drawcount;

/* globals */
double normalizedh;
int normalizedh2;

GLsizei wh = 500, ww = 500; /* initial window size */
GLsizei ih = 500, iw = 500;
GLfloat size = 3.0;   /* half side length of square */
int draw_mode = 0; /* drawing mode */
int rx, ry; /*raster position*/

GLfloat r = 1.0, g = 1.0, b = 1.0; /* drawing color */

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
    normalizedh = (double) 500.0 - (double) 0.5 * (double) h; //(double) h;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, w, -h/2.0 + normalizedh, h/2.0 + normalizedh, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 

    glClearColor (0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    display(GL_RENDER);
    glFlush();
    
    // berikan informasi panjang dan lebar saat ini
    // untuk dipergunakan fungsi lain
    ww = w;
    wh = h; 
    normalizedh2 = 500 - h;
}

// type: tipe objeknya. Length: berapa kali objmAddPosition akan dipanggil
void objmAddObject(int type, int length) {
    objtypes     [totalobj] = type;
    objtypelength[totalobj] = length;
    objposoffset [totalobj] = nextoffset;

    float r, g, b;
    setcolor(activecolorenum);
    r = ccolorr; b = ccolorb; g = ccolorg;

    objfilled    [totalobj] = fill;
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

    // Inisialisasi nilai untuk semuanya
    totalobj = 0;
    nextoffset = 0;
}

void drawObject(GLenum mode) {
    int iteratedpos = 0;
    int baseindex = 0;
    for (int i = 0; i < totalobj; i++) {
        // Atur warnanya dulu
        glColor3f(objcolorr[i], objcolorg[i], objcolorb[i]);
        baseindex = objposoffset[i];
        switch (objtypes[i]) {
            case LINE:
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
                if (objfilled[i] == true)
                    glBegin(GL_POLYGON);
                else
                    glBegin(GL_LINE_LOOP);

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
                glPointSize(20.0);
                glBegin(GL_POINTS);
                glVertex2f((GLfloat) objpositions[baseindex], (GLfloat) objpositions[baseindex + 1]);
                glEnd();
                break;
        }
    }
    glFlush();
}

#define SIZE 512

void mouse(int btn, int state, int x, int y)
{
    
    int where;
    static int xp[99],yp[99];
    if(btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN) 
    {
        GLuint selectBuf[SIZE];
        GLint hits;
        GLint viewport[4];

        glGetIntegerv (GL_VIEWPORT, viewport);

        glSelectBuffer (SIZE, selectBuf);
        glRenderMode(GL_SELECT);

        glInitNames();
        glPushName(0);

        glMatrixMode (GL_PROJECTION);
        glPushMatrix ();
        glLoadIdentity ();
    
        gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 
                        5.0, 5.0, viewport);
        glOrtho( 0, ww, -wh/2.0 + normalizedh, wh/2.0 + normalizedh, -1, 1);
        display(GL_SELECT);

        glMatrixMode (GL_PROJECTION);
        glPopMatrix ();

        hits  = glRenderMode (GL_RENDER);
        where = pick(hits, selectBuf);
        glutPostRedisplay();
 
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        printf("Drawmode: %d", where);
        if(where != 0 && where <= EDITMODE)
        {
            drawcount = 0;
            draw_mode = where;
        }
        else if (drawcount < 0) {
            drawcount = 0;
        }
        else if (draw_mode <= POLYGON)  {
            printf("Entering drawing point note stage with %d\n", draw_mode);
            switch(draw_mode) {
        case(LINE):
            if(drawcount<=0) {
                drawcount = 1;
                xp[0] = x;
                yp[0] = y;
            } else {
                objmAddObject(LINE, 2);
                objmAddPosition(xp[0], wh - yp[0] + normalizedh2);
                objmAddPosition(x    , wh - y + normalizedh2);
                
                drawcount=0;
            }
            break;
        case(RECTANGLE):
            if(drawcount <= 0) {
                drawcount = 1;
                xp[0] = x;
                yp[0] = y;
            } else {
                objmAddObject(POLYGON, 4);
                objmAddPosition(x, wh-y + normalizedh2);
                objmAddPosition(x, wh-yp[0] + normalizedh2);
                objmAddPosition(xp[0], wh-yp[0] + normalizedh2);
                objmAddPosition(xp[0], wh-y + normalizedh2);
                drawcount=0;
            }
            break;
        case (TRIANGLE):
            switch(drawcount)
            {
            case(-1):
                drawcount=1;
                xp[0] = x;
                yp[0] = y;
                break;
            case(0):
                drawcount++;
                xp[0] = x;
                yp[0] = y;
                break;
            case(1):
                drawcount++;
                xp[1] = x;
                yp[1] = y;
                break;
            case(2):
                objmAddObject(POLYGON, 3);
                objmAddPosition(xp[0],wh-yp[0] + normalizedh2);
                objmAddPosition(xp[1],wh-yp[1] + normalizedh2);
                objmAddPosition(x,wh-y + normalizedh2);
                drawcount=0;
            }
            break;
        case(POINTS):
            {
                objmAddObject(POINTS, 1);
                objmAddPosition(x,wh-y + normalizedh2);
                drawcount++;
            }
            break;
        case(POLYGON): 
            {
            xp[drawcount] = x;
            yp[drawcount] = y;
            drawcount++;
            }
        case(TEXT): 
            {
                rx=x;
                ry=wh-y;
                glRasterPos2i(rx,ry); 
                drawcount=0;
            }
        }
        }
        glPopAttrib();
        glFlush();
    }
    else if(btn==GLUT_RIGHT_BUTTON && state==GLUT_DOWN) {

    }
    drawObject(GL_RENDER);
}

int pick (GLint nPicks, GLuint pickBuffer[])
{
    int j;
    unsigned int k, lm;
    GLuint objID, *ptr;
    lm = 0;
    printf (" Number of objects picked = %d\n", nPicks);
    printf ("\n");
    ptr = pickBuffer;

    // If nothing picked, just return zero
    if (nPicks == 0) return 0;

    /*  Output all items in each pick record.  */
    for (j = 0; j < nPicks; j++) {
        objID = *ptr;
        ptr += 3;
        for (k = 0; k < objID; k++) {
            printf ("   %d ",*ptr);
            if (*ptr != 0) lm = *ptr;
            ptr++;
        }
        printf ("\n\n");
    }

    // Kasus 1: LM adalah pemilihan warna
    if (lm > EDITMODE) {
        printf("Color set: %d\n", lm);
        setcolor(lm); // Begini saja
        drawcount = -1;
        lm = activecolorenum;
    }

    // Dapatkan kontrol terakhir yang bukan nol
    return lm;
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

// Menggambar tanpa isi
void screen_box2(int x_, int y_, int s_ )
{
    int s = s_ - 10;
    int x = x_ + 5;
    int y = y_ + 5;

    glColor3f(0.4, 0.4, 0.4);
    glLineWidth(5.0);
    glBegin(GL_LINE_LOOP);
      glVertex2i(x, y);
      glVertex2i(x+s, y);
      glVertex2i(x+s, y+s);
      glVertex2i(x, y+s);
    glEnd();
    glLineWidth(1.0);
    glColor3f(0.7, 0.7, 0.7);
}

void right_menu(int id)
{
   if(id == 1) exit(0);
   else display(GL_RENDER);
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
   rx+=glutBitmapWidth(GLUT_BITMAP_9_BY_15,k);
   glFlush();
}

void display(GLenum mode)
{
	int shift=0;
    // Toolbox Redraw
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClearColor (0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Warna untuk latar belakang
    glColor3f(0.7, 0.7, 0.7);

    // Toolbox Background
    if (mode == GL_SELECT) glLoadName(LINE);
    screen_box(0,ih-iw/10,iw/10);
    if (draw_mode == LINE) screen_box2(0,ih-iw/10,iw/10);

    if (mode == GL_SELECT) glLoadName(RECTANGLE);
    screen_box(iw/10,ih-iw/10,iw/10);
    if (draw_mode == RECTANGLE) screen_box2(iw/10,ih-iw/10,iw/10);

    if (mode == GL_SELECT) glLoadName(TRIANGLE);
    screen_box(iw/5,ih-iw/10,iw/10);
    if (draw_mode == TRIANGLE) screen_box2(iw/5,ih-iw/10,iw/10);

    if (mode == GL_SELECT) glLoadName(POINTS);
    screen_box(3*iw/10,ih-iw/10,iw/10);
    if (draw_mode == POINTS) screen_box2(3*iw/10,ih-iw/10,iw/10);

    if (mode == GL_SELECT) glLoadName(TEXT);
    screen_box(2*iw/5,ih-iw/10,iw/10);
    if (draw_mode == TEXT) screen_box2(2*iw/5,ih-iw/10,iw/10);

    if (mode == GL_SELECT) glLoadName(POLYGON);
    screen_box(5*iw/10,ih-iw/10,iw/10);
    if (draw_mode == POLYGON) screen_box2(5*iw/10,ih-iw/10,iw/10);

    if (mode == GL_SELECT) glLoadName(EDITMODE);
    screen_box(6*iw/10,ih-iw/10,iw/10);
    if (draw_mode == EDITMODE) screen_box2(6*iw/10,ih-iw/10,iw/10);

    // Color box. SPARTAAA!
    if (mode == GL_SELECT) glLoadName(COLOR1);
    glColor3f(0.7, 0.7, 0.7);
    screen_box(0,normalizedh2,iw/10);
    if (activecolorenum == COLOR1) screen_box2(0,normalizedh2,iw/10);
    setcolor2(COLOR1, true);
    screen_box(10,10 + normalizedh2,iw/10 - 20);

    if (mode == GL_SELECT) glLoadName(COLOR2);
    glColor3f(0.7, 0.7, 0.7);
    screen_box(iw/10,normalizedh2,iw/10);
    if (activecolorenum == COLOR2) screen_box2(iw/10,normalizedh2,iw/10);
    setcolor2(COLOR2, true);
    screen_box(10+iw/10,10 + normalizedh2,iw/10 - 20);

    if (mode == GL_SELECT) glLoadName(COLOR3);
    glColor3f(0.7, 0.7, 0.7);
    screen_box(2*iw/10,normalizedh2,iw/10);
    if (activecolorenum == COLOR3) screen_box2(2*iw/10,normalizedh2,iw/10);
    setcolor2(COLOR3, true);
    screen_box(10+2*iw/10,10 + normalizedh2,iw/10 - 20);

    if (mode == GL_SELECT) glLoadName(COLOR4);
    glColor3f(0.7, 0.7, 0.7);
    screen_box(3*iw/10,normalizedh2,iw/10);
    if (activecolorenum == COLOR4) screen_box2(3*iw/10,normalizedh2,iw/10);
    setcolor2(COLOR4, true);
    screen_box(10+3*iw/10,10 + normalizedh2,iw/10 - 20);

    if (mode == GL_SELECT) glLoadName(COLOR5);
    glColor3f(0.7, 0.7, 0.7);
    screen_box(4*iw/10,normalizedh2,iw/10);
    if (activecolorenum == COLOR5) screen_box2(4*iw/10,normalizedh2,iw/10);
    setcolor2(COLOR5, true);
    screen_box(10+4*iw/10,10 + normalizedh2,iw/10 - 20);

    if (mode == GL_SELECT) glLoadName(COLORNOFILL);
    glColor3f(0.7, 0.7, 0.7);
    screen_box(5*iw/10,normalizedh2,iw/10);
    if (fill == false) screen_box2(5*iw/10,normalizedh2,iw/10);
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_LINE_LOOP);
        glVertex2i(5*iw/10 + 10, 10);
        glVertex2i(5*iw/10 + 10, iw/10 - 10);
        glVertex2i(6*iw/10 - 10, iw/10 - 10);
        glVertex2i(6*iw/10 - 10, 10);
    glEnd();

    // Reset select mode
    if (mode == GL_SELECT) {
        glPopName();
    }

    // Warna untuk objek
    glColor3f(0.4, 0.4, 0.4);
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
	glRasterPos2i(2*iw/5 + 7,ih-iw/20);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'A');
	shift=glutBitmapWidth(GLUT_BITMAP_9_BY_15, 'A');
	glRasterPos2i(2*iw/5+shift+7,ih-iw/20);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'B');
	shift+=glutBitmapWidth(GLUT_BITMAP_9_BY_15, 'B');
	glRasterPos2i(2*iw/5+shift+7,ih-iw/20);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'C');

    glColor3f(0.4, 0.4, 0.4);
    glBegin(GL_POLYGON);
        glVertex2i(5*iw/10,ih-iw/10 + 10);
        glVertex2i(5*iw/10 + 20,ih-iw/10 + 50);
        glVertex2i(5*iw/10 + 50,ih-iw/10 + 30);
        glVertex2i(5*iw/10 + 40,ih-iw/10 + 10);
        glVertex2i(5*iw/10 + 10,ih-iw/10 + 20);
    glEnd();

    glFlush();
    glPopAttrib();

    // Object redraw
    drawObject(mode);
}
void setcolor2(int coloren, bool failsafe) {
    switch (coloren) {
    case COLOR1:
        ccolorr = 0.2; ccolorg = 0.5; ccolorb = 0.7;
        break;
    case COLOR2:
        ccolorr = 0.4; ccolorg = 0.3; ccolorb = 0.6;
        break;
    case COLOR3:
        ccolorr = 0.7; ccolorg = 0.5; ccolorb = 0.7;
        break;
    case COLOR4:
        ccolorr = 0.2; ccolorg = 0.7; ccolorb = 0.2;
        break;
    case COLOR5:
        ccolorr = 0.7; ccolorg = 0.4; ccolorb = 0.4;
        break;
    case COLORNOFILL:
        fill = !fill;
        printf("Fill now %s\n", fill ? "ON" : "OFF");
        break;
    }
    if (!failsafe && coloren != COLORNOFILL) activecolorenum = coloren;
    glColor3f(ccolorr, ccolorg, ccolorb);   
}
void setcolor(int coloren) {
    setcolor2(coloren, false);
}

void displaywrap() {
    display(GL_RENDER);
}

int main(int argc, char** argv)
{
    int c_menu, p_menu, f_menu;

    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
    glutCreateWindow("FidPaint - Powered by GLUT | Worksheet 1 Computer Graphic");
    glutDisplayFunc(displaywrap);
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