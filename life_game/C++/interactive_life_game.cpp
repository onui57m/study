/**
 *
 * Use OpenGL to visualize
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-04T03:33:22+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-04T19:06:21+09:00
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <random>

#include <GL/gl.h>
#include <GL/glut.h>

typedef std::vector<int> vi;
typedef std::vector<vi> vvi;
typedef std::vector<double> vd;

#define POINT_SIZE 5

int width, height, boundary;
int init_seed;
char move_flag;
vvi field, pre_field;
vd interactive_pos(2);

vi init_window_position(2);
int window_width, window_height;

void make_init(vvi &field, int width, int height);
vvi calc_react(vvi &pre_field, int width, int height, int boundary);

/* ++++++++++ openGL ++++++++++ */
void initialize();
void display();
void timer(int a);
void keyboard(unsigned char key, int a, int b);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
/* ++++++++++ openGL ++++++++++ */

int main(int argc, char *argv[])
{
  std::ifstream ifs;
  char file_name[256];
  vi tmp;

  sprintf(file_name,"param.dat");
  ifs.open(file_name);
  if (!ifs)
  {
    std::cout << "Cannot open " << file_name << ".\n";
    return 1;
  }
  ifs >> width >> height >> boundary;
  ifs.close();

  for (int i = 0; i < height+2; i++)
    tmp.push_back(0);
  for (int i = 0; i < width+2; i++)
    field.push_back(tmp);

  make_init(field, width, height);
  pre_field = field;

  /* ++++++++++ openGL ++++++++++ */
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
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutTimerFunc(100, timer, 0);
  initialize();
  glutMainLoop();
  /* ++++++++++ openGL ++++++++++ */
}

void make_init(vvi &field, int width, int height)
{
  std::mt19937 mt(init_seed);
  std::uniform_int_distribution<int> dist(0, 1);

  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      field.at(i).at(j) = dist(mt);
    }
  }
}
vvi calc_react(vvi &pre_field, int width, int height, int boundary)
{
  vvi field(width+2, vi(height+2));
  int count;

  if (boundary == 0)
  {
    for (int i = 0; i < height+2; i++)
    {
      pre_field.at(0).at(i) = 0;
      pre_field.at(width+1).at(i) = 0;
    }
    for (int i = 0; i < width+2; i++)
    {
      pre_field.at(i).at(0) = 0;
      pre_field.at(i).at(height+1) = 0;
    }
  }
  else if (boundary == 1)
  {
    for (int i = 1; i <= height; i++)
    {
      pre_field.at(0).at(i) = pre_field.at(width).at(i);
      pre_field.at(width+1).at(i) = pre_field.at(1).at(i);
    }
    for (int i = 1; i <= width; i++)
    {
      pre_field.at(i).at(0) = pre_field.at(i).at(height);
      pre_field.at(i).at(height+1) = pre_field.at(i).at(1);
    }
    pre_field.at(0).at(0) = pre_field.at(width).at(0);
    pre_field.at(width+1).at(0) = pre_field.at(1).at(0);
    pre_field.at(0).at(height+1) = pre_field.at(width).at(height+1);
    pre_field.at(width+1).at(height+1) = pre_field.at(1).at(height+1);
  }

  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      count = 0;
      count += pre_field.at(i-1).at(j-1);
      count += pre_field.at(i-1).at(j);
      count += pre_field.at(i-1).at(j+1);
      count += pre_field.at(i).at(j-1);
      count += pre_field.at(i).at(j+1);
      count += pre_field.at(i+1).at(j-1);
      count += pre_field.at(i+1).at(j);
      count += pre_field.at(i+1).at(j+1);
      if (pre_field.at(i).at(j))
      {
        if (count <= 1 || count >= 4)
          field.at(i).at(j) = 0;
        else
          field.at(i).at(j) = 1;
      }
      else
      {
        if (count == 3)
          field.at(i).at(j) = 1;
        else
          field.at(i).at(j) = 0;
      }
    }
  }
  return field;
}

/* ++++++++++ openGL ++++++++++ */
void initialize()
{
  init_seed = 1;
  move_flag = 'p';

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST);
}
void display()
{
  // printf("| == in display == ");
  if (move_flag == 's')
  {
    field = calc_react(pre_field, width, height, boundary);
    pre_field = field;
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPointSize(POINT_SIZE);
  glBegin(GL_POINTS);
  glColor3f(0 , 0 , 0);
  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      if (field.at(i).at(j))
      glVertex2f((i-0.5)/(width/2.)-1, -((j-0.5)/(height/2.)-1));
    }
  }
  glEnd();
  glutSwapBuffers();
}
void timer(int a)
{
  // printf("| == in timer == ");
	glutPostRedisplay();
	glutTimerFunc(50 , timer , 0);
}
void keyboard(unsigned char key, int a, int b)
{
  // printf("| == in keyboard == ");
  printf("%c: ", key);
  switch (key)
  {
    case 'p':
      move_flag = 'p';
      printf("posing...\n");
      break;
    case 's':
      move_flag = 's';
      printf("start!\n");
      break;
    case 'i':
      init_seed++;
      make_init(field, width, height);
      pre_field = field;
      printf("initialize\n");
      break;
    case 'c':
      for (int i = 1; i <= width; i++)
      {
        for (int j = 1; j <= height; j++)
        {
          field.at(i).at(j) = 0;
        }
      }
      pre_field = field;
      printf("clear\n");
      break;
    case 'q':
      printf("quit\n");
      exit(0);
      break;
    default:
      printf("no action\n");
      break;
  }
}
void mouse(int button, int state, int x, int y)
{
  // printf("| == in mouse == ");
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      if (move_flag != 'p')
      {
        move_flag = 'p';
        printf("posing...\n");
      }
      break;
    case GLUT_RIGHT_BUTTON:
      if (move_flag != 's')
      {
        move_flag = 's';
        printf("start!\n");
      }
      break;
    default:
      break;
  }
}
void motion(int x, int y)
{
  field.at(x/5).at(y/5) = 1;
  pre_field.at(x/5).at(y/5) = 1;
}
/* ++++++++++ openGL ++++++++++ */
