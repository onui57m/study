/**
 *
 * Use OpenGL to visualize
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-04T03:33:22+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-04T18:52:54+09:00
 */

#include <fstream>
#include <cstdio>
#include <vector>

#include <GL/gl.h>
#include <GL/glut.h>

typedef std::vector<int> vi;

#define POINT_SIZE 5

void initialize();
void display();
void timer(int a);
void keyboard(unsigned char key, int a, int b);

int vis_index = 0;
int width, height, point_size;
int total_step;
char move_flag;

vi init_window_position(2);
int window_width, window_height;

int main(int argc, char *argv[])
{
  std::ifstream param;
  int width, height;
  param.open("param.dat");
  param >> width >> height;
  param.close();

  init_window_position.at(0) = 100;
  init_window_position.at(1) = 100;
  window_width = width*POINT_SIZE;
  window_height = height*POINT_SIZE;
  char title[] = "life game";

  glutInit(&argc, argv);
  glutInitWindowPosition(init_window_position.at(0), init_window_position.at(1));
  glutInitWindowSize(window_width, window_height);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(title);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(100, timer, 0);
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
  move_flag = 'p';

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
    glPointSize(POINT_SIZE);
    glBegin(GL_POINTS);
    glColor3f(0 , 0 , 0);
    for (int i = 1; i <= width; i++)
    {
      for (int j = 1; j <= height; j++)
      {
        ifs >> cell;
        if (cell)
        glVertex2f((i-0.5)/(width/2.)-1, -((j-0.5)/(height/2.)-1));
      }
    }
    glEnd();
    glutSwapBuffers();

    ifs.close();
    if (move_flag == 's')
      vis_index++;
  }
}
void timer(int a)
{
	glutPostRedisplay();
	glutTimerFunc(50 , timer , 0);
}
void keyboard(unsigned char key, int a, int b)
{
  printf("%c", key);
  fflush(stdout);
  switch (key)
  {
    case 'i':
      vis_index = 0;
      break;
    case 'e':
      vis_index = total_step-1;
      break;
    case 'f':
      if (vis_index < total_step-1)
        vis_index++;
      break;
    case 'b':
      if (vis_index > 0)
        vis_index--;
      break;
    case 'p':
      move_flag = 'p';
      break;
    case 's':
      move_flag = 's';
      break;
    case 'q':
      printf("\nquit\n");
      exit(0);
      break;
    default:
      printf("\n%c: no action\n",key);
      break;
  }
}
