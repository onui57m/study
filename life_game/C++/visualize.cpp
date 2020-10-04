/**
 *
 * Use OpenGL to visualize
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-04T03:33:22+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-04T14:19:04+09:00
 */

#include <fstream>
#include <cstdio>
#include <GL/gl.h>
#include <GL/glut.h>

void initialize();
void display();
void delay(int a);
void keyboard(unsigned char key, int a, int b);

int vis_index = 0;
int width, height, point_size;
int total_step;

int main(int argc, char *argv[])
{
  std::ifstream param;
  int width, height;
  param.open("param.dat");
  param >> width >> height;
  param.close();

  int WindowPositionX = 100;
  int WindowPositionY = 100;
  int WindowWidth = width*5;
  int WindowHeight = height*5;
  char WindowTitle[] = "life game";

  glutInit(&argc, argv);
  glutInitWindowPosition(WindowPositionX, WindowPositionY);
  glutInitWindowSize(WindowWidth, WindowHeight);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(WindowTitle);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(100, delay, 0);
  initialize();
  glutMainLoop();
}

void initialize()
{
  std::ifstream param;
  int dummy;
  param.open("param.dat");
  param >> width >> height >> dummy;
  param >> total_step;
  param.close();
  point_size = width/20;

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST);
}
void display()
{
  char vis_file[256];
  std::ifstream ifs;
  int cell;

  if (vis_index < total_step)
  {
    std::sprintf(vis_file,"dat/state_%03d.dat",vis_index);
    ifs.open(vis_file);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(point_size);
    glBegin(GL_POINTS);
    glColor3f(0 , 0 , 0);
    for (int i = 1; i <= width; i++)
    {
      for (int j = 1; j <= height; j++)
      {
        ifs >> cell;
        if (cell)
        glVertex2f(i/(width/2.)-1, j/(height/2.)-1);
      }
    }
    glEnd();
    glutSwapBuffers();

    ifs.close();
    vis_index++;
  }
}
void delay(int a)
{
	glutPostRedisplay();
	glutTimerFunc(50 , delay , 0);
}
void keyboard(unsigned char key, int a, int b)
{
  printf("%hhu", key);
  switch (key)
  {
    case 's':
      vis_index = 0;
      break;
    case 'e':
      vis_index = total_step-1;
      break;
    default:
     break;
  }
}
