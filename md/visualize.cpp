/**
 *
 * use OpenGL to visualize
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-07T00:53:13+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-07T05:09:36+09:00
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <GL/gl.h>
#include <GL/glut.h>

#define S_LJ 1.
#define WIDTH 500
#define HEIGHT 500
#define PI 3.141592653589793238462
#define TIME_LINE_BYTE 13
#define ONE_LINE_BYTE 39

typedef std::vector<int> vi;
typedef std::vector<double> vd;

int par_num, total_step, log_step;
double box_x, box_y, box_z, dt, current_time;
vd pos_x, pos_y, pos_z;

int active_window;
vi window_id(2);
int base_pos;
char move_flag, orth_flag;
std::ifstream::off_type traj_off;
std::ifstream::pos_type traj_pos;
std::ifstream ftraj;

void read_cood();
int get_traj_off(int offset);

void initialize();
void display0();
void timer(int a);
void keyboard(unsigned char key, int a, int b);
void mouse(int button, int state, int x, int y);
void display1();

void write_string(double x, double y, std::string str);

int main(int argc, char *argv[])
{
  std::ifstream ifs, fparam;

  fparam.open("param.dat");
  if (!fparam)
  {
    std::cout << "Cannot open param.dat.\n";
    exit(1);
  }
  fparam >> total_step >> log_step >> dt;
  fparam.close();

  ifs.open("init.dat");
  if(!ifs)
  {
    std::cout << "Cannot open init.dat.\n";
    exit(1);
  }
  ifs >> par_num;
  ifs.close();

  ftraj.open("traj.dat");
  if(!ftraj)
  {
    std::cout << "Cannot open traj.dat.\n";
    exit(1);
  }

  for (int i = 0; i < par_num; i++)
  {
    pos_x.push_back(0.0);
    pos_y.push_back(0.0);
    pos_z.push_back(0.0);
  }

  /* ++++++++++ OpenGL ++++++++++ */
  char title0[] = "visualize";
  char title1[] = "graph";

  glutInit(&argc, argv);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  window_id.at(0) = glutCreateWindow(title0);
  glutDisplayFunc(display0);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);

  glutInitWindowPosition(800, 100);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  window_id.at(1) = glutCreateWindow(title1);
  glutDisplayFunc(display1);

  glutTimerFunc(100, timer, 0);
  initialize();
  glutMainLoop();
  /* ++++++++++ OpenGL ++++++++++ */
}

/* ++++++++++ functions ++++++++++ */
void read_cood()
{
  if (base_pos)
    ftraj.seekg(traj_off,std::ios_base::cur);
  else
    ftraj.seekg(traj_off,std::ios_base::beg);
  traj_pos = ftraj.tellg();

  ftraj >> current_time;
  ftraj >> box_x >> box_y >> box_z;
  for (int i = 0; i < par_num; i++)
  {
    ftraj >> pos_x.at(i) >> pos_y.at(i) >> pos_z.at(i);
  }
  if (ftraj.eof())
  {
    ftraj.clear();
    ftraj.seekg(traj_pos);
  }
  if (move_flag == 'p')
  {
    traj_off = 0;
    base_pos = 1;
    ftraj.seekg(traj_pos);
  }
}
int get_traj_off(int offset)
{
  int now_step, rest;

  if (move_flag != 'p') current_time += log_step*dt;
  now_step = current_time/dt;
  if (offset > 0)
  {
    rest = (total_step - now_step)/log_step;
    if (offset > rest) offset = rest;
  }
  else if (offset < 0)
  {
    rest = now_step/log_step;
    if (offset < -rest) offset = -rest;
  }

  return (TIME_LINE_BYTE+ONE_LINE_BYTE*(par_num+1))*offset;
}

/* ++++++++++ OpenGL ++++++++++ */
void initialize()
{
  move_flag = 'p';
  orth_flag = 'x';
  traj_off = 0;
  base_pos = 0;
  ftraj.seekg(0,std::ios_base::beg);
  traj_pos = ftraj.tellg();
  active_window = glutGetWindow();

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST);
}
void display0()
{
  char now[256];

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  read_cood();

  glPointSize(5);
  glColor3f(1, 0, 0);
  glBegin(GL_POINTS);
  for (int i = 0; i < par_num; i++)
  {
    if (orth_flag == 'x')
      glVertex2f(2.*pos_y.at(i)/box_y, 2.*pos_z.at(i)/box_z);
    else if (orth_flag == 'y')
      glVertex2f(2.*pos_z.at(i)/box_z, 2.*pos_x.at(i)/box_x);
    else if (orth_flag == 'z')
      glVertex2f(2.*pos_x.at(i)/box_x, 2.*pos_y.at(i)/box_y);
  }
  glEnd();

  glutSwapBuffers();
}
void timer(int a)
{
  glutSetWindow(window_id.at(0));
	glutPostRedisplay();
  glutSetWindow(window_id.at(1));
  glutPostRedisplay();

	glutTimerFunc(50 , timer , 0);
}
void keyboard(unsigned char key, int a, int b)
{
  active_window = glutGetWindow();

  if (active_window == window_id.at(0))
  {
    switch (key)
    {
      case 'p':
        move_flag = 'p';
        break;
      case 's':
        base_pos = 1;
        traj_off = 0;
        move_flag = 's';
        break;
      case 'i':
        base_pos = 0;
        traj_off = 0;
        move_flag = 'p';
        break;
      case 'e':
        base_pos = 0;
        traj_off = get_traj_off(total_step/log_step);
        break;
      case 'f':
        base_pos = 1;
        traj_off = get_traj_off(1);
        break;
      case 'F':
        base_pos = 1;
        traj_off = get_traj_off(5);
        break;
      case 'b':
        base_pos = 1;
        traj_off = get_traj_off(-1);
        break;
      case 'B':
        base_pos = 1;
        traj_off = get_traj_off(-5);
        break;
      case 'q':
        ftraj.close();
        printf("quit\n");
        exit(0);
        break;
      case 'x':
        orth_flag = 'x';
        break;
      case 'y':
        orth_flag = 'y';
        break;
      case 'z':
        orth_flag = 'z';
        break;
      default:
        printf("no action\n");
        break;
    }
  }
}
void mouse(int button, int state, int x, int y)
{
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      if (move_flag != 'p')
        move_flag = 'p';
      break;
    case GLUT_RIGHT_BUTTON:
      if (move_flag != 's')
        base_pos = 1;
        traj_off = 0;
        move_flag = 's';
      break;
    default:
      break;
  }
}

void display1()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glutSwapBuffers();
}

void write_string(double x, double y, std::string str)
{
  glColor3f(0, 0, 0);
  glRasterPos2f(x, y);

  for (int i = 0; i < str.size(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[i]);
}
