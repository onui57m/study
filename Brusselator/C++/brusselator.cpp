/**
 *
 * Use OpenGL to visualize
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-04T22:19:25+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-17T03:32:00+09:00
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <random>
#include <cmath>
#include <algorithm>

#include <GL/gl.h>
#include <GL/glut.h>

typedef std::vector<int> vi;
typedef std::vector<double> vd;
typedef std::vector<vd> vvd;

#define A 1.
#define B 4.
#define DU 10.
#define ETA 0.4
#define DV (DU/(ETA*ETA))

#define DT 0.001

#define POINT_SIZE 5.
#define LINE_WIDTH 5.
#define COLOR_BAR_WIDTH 100.
#define TIME_BAR 50.
#define NUM_SEP 100

#define PI 3.141592653589793

int width, height, boundary;
int init_seed, istep;
double current_time;
char move_flag, color_flag;
int active_window;
vvd field_u, field_v, pre_field_u, pre_field_v;
vd interactive_pos(2);

vi window_id(2), init_window0_position(2), init_window1_position(2);
vi window0_size(2), window1_size(2);

void make_init();
void calc_react();

/* ++++++++++ openGL ++++++++++ */
void initialize();
void display0();
void timer(int a);
void keyboard(unsigned char key, int a, int b);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void display1();

void write_string(double x, double y, char str[]);
void write_color_bar(double max_p, double min_p);
/* ++++++++++ openGL ++++++++++ */

int main(int argc, char *argv[])
{
  std::ifstream ifs;
  char file_name[256];
  vd tmp;

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
  {
    field_u.push_back(tmp);
    field_v.push_back(tmp);
  }

  make_init();
  pre_field_u = field_u;
  pre_field_v = field_v;
  istep = 0;
  current_time = 0.0;

  /* ++++++++++ openGL ++++++++++ */
  init_window0_position.at(0) = 100;
  init_window0_position.at(1) = 100;
  init_window1_position.at(0) = 800;
  init_window1_position.at(1) = 100;
  window0_size.at(0) = width*POINT_SIZE + COLOR_BAR_WIDTH;
  window0_size.at(1) = height*POINT_SIZE + TIME_BAR;
  window1_size.at(0) = window0_size.at(0);
  window1_size.at(1) = window0_size.at(1);
  char title0[] = "Brusselator";
  char title1[] = "graph";

  glutInit(&argc, argv);

  glutInitWindowPosition(init_window0_position.at(0), init_window0_position.at(1));
  glutInitWindowSize(window0_size.at(0), window0_size.at(1));
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  window_id.at(0) = glutCreateWindow(title0);
  glutDisplayFunc(display0);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  glutInitWindowPosition(init_window1_position.at(0), init_window1_position.at(1));
  glutInitWindowSize(window1_size.at(0), window1_size.at(1));
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  window_id.at(1) = glutCreateWindow(title1);
  glutDisplayFunc(display1);

  glutTimerFunc(100, timer, 0);
  initialize();
  glutMainLoop();
  /* ++++++++++ openGL ++++++++++ */
}

void make_init()
{
  std::mt19937 mt(init_seed);
  std::uniform_real_distribution<double> dist(-1, 1);

  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      field_u.at(i).at(j) = A + dist(mt)/2.;
      field_v.at(i).at(j) = B/A + dist(mt)/2.;
    }
  }
}
void calc_react()
{
  vvd div_u(width+2, vd(height+2, 0.));
  vvd div_v(width+2, vd(height+2, 0.));
  double tmp;

  if (boundary == 0)
  {
    for (int i = 0; i < height+2; i++)
    {
      pre_field_u.at(0).at(i) = 0;
      pre_field_u.at(width+1).at(i) = 0;
      pre_field_v.at(0).at(i) = 0;
      pre_field_v.at(width+1).at(i) = 0;
    }
    for (int i = 0; i < width+2; i++)
    {
      pre_field_u.at(i).at(0) = 0;
      pre_field_u.at(i).at(height+1) = 0;
      pre_field_v.at(i).at(0) = 0;
      pre_field_v.at(i).at(height+1) = 0;
    }
  }
  else if (boundary == 1)
  {
    for (int i = 1; i <= height; i++)
    {
      pre_field_u.at(0).at(i) = pre_field_u.at(width).at(i);
      pre_field_u.at(width+1).at(i) = pre_field_u.at(1).at(i);
      pre_field_v.at(0).at(i) = pre_field_v.at(width).at(i);
      pre_field_v.at(width+1).at(i) = pre_field_v.at(1).at(i);
    }
    for (int i = 1; i <= width; i++)
    {
      pre_field_u.at(i).at(0) = pre_field_u.at(i).at(height);
      pre_field_u.at(i).at(height+1) = pre_field_u.at(i).at(1);
      pre_field_v.at(i).at(0) = pre_field_v.at(i).at(height);
      pre_field_v.at(i).at(height+1) = pre_field_v.at(i).at(1);
    }
    pre_field_u.at(0).at(0) = pre_field_u.at(width).at(0);
    pre_field_u.at(width+1).at(0) = pre_field_u.at(1).at(0);
    pre_field_u.at(0).at(height+1) = pre_field_u.at(width).at(height+1);
    pre_field_u.at(width+1).at(height+1) = pre_field_u.at(1).at(height+1);
    pre_field_v.at(0).at(0) = pre_field_v.at(width).at(0);
    pre_field_v.at(width+1).at(0) = pre_field_v.at(1).at(0);
    pre_field_v.at(0).at(height+1) = pre_field_v.at(width).at(height+1);
    pre_field_v.at(width+1).at(height+1) = pre_field_v.at(1).at(height+1);
  }

  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      div_u.at(i).at(j) = pre_field_u.at(i-1).at(j) + pre_field_u.at(i+1).at(j);
      div_u.at(i).at(j) += pre_field_u.at(i).at(j-1) + pre_field_u.at(i).at(j+1);
      div_u.at(i).at(j) -= 4*pre_field_u.at(i).at(j);
      div_v.at(i).at(j) = pre_field_v.at(i-1).at(j) + pre_field_v.at(i+1).at(j);
      div_v.at(i).at(j) += pre_field_v.at(i).at(j-1) + pre_field_v.at(i).at(j+1);
      div_v.at(i).at(j) -= 4*pre_field_v.at(i).at(j);

      tmp = -B*pre_field_u.at(i).at(j) + pre_field_u.at(i).at(j)*pre_field_u.at(i).at(j)*pre_field_v.at(i).at(j);
      field_u.at(i).at(j) += (DU*div_u.at(i).at(j) + A + tmp - pre_field_u.at(i).at(j))*DT;
      field_v.at(i).at(j) += (DV*div_v.at(i).at(j) - tmp)*DT;
    }
  }
  istep++;
  current_time += istep*DT;
}

/* ++++++++++ openGL ++++++++++ */
void initialize()
{
  init_seed = 1;
  move_flag = 'p';
  color_flag = 'u';
  active_window = glutGetWindow();

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST);
}
void display0()
{
  double r, g, b;
  double max_u, max_v;
  double min_u, min_v;
  char now[256];

  r = 0.0, g = 0.0, b = 0.0;
  max_u = 0.0, max_v = 0.0;
  min_u = 10.0, min_v = 10.0;
  if (move_flag == 's')
  {
    calc_react();
    pre_field_u = field_u;
    pre_field_v = field_v;
  }
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  glPointSize(POINT_SIZE);
  glBegin(GL_POINTS);
  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      if (field_u.at(i).at(j) > max_u)
        max_u = field_u.at(i).at(j);
      if (field_u.at(i).at(j) < min_u)
        min_u = field_u.at(i).at(j);
      if (field_v.at(i).at(j) > max_v)
        max_v = field_v.at(i).at(j);
      if (field_v.at(i).at(j) < min_v)
        min_v = field_v.at(i).at(j);
    }
  }
  max_u = std::ceil(max_u*10)/10.;
  max_v = std::ceil(max_v*10)/10.;
  min_u = std::floor(min_u*10)/10.;
  min_v = std::floor(min_v*10)/10.;

  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
    {
      if (color_flag == 'u')
      {
        r = std::min(1., std::fabs(2*(field_u.at(i).at(j)-min_u)/(max_u-min_u) - 0.5));
        g = std::sin(PI*(field_u.at(i).at(j)-min_u)/(max_u-min_u));
        b = std::cos(PI/2.*(field_u.at(i).at(j)-min_u)/(max_u-min_u));
      }
      else if (color_flag == 'v')
      {
        r = std::min(1., std::fabs(2*(field_v.at(i).at(j)-min_v)/(max_v-min_v) - 0.5));
        g = std::sin(PI*(field_v.at(i).at(j)-min_v)/(max_v-min_v));
        b = std::cos(PI/2.*(field_v.at(i).at(j)-min_v)/(max_v-min_v));
      }
      glColor3f(r, g, b);
      glVertex2f((i-0.5)/(window0_size.at(0)/(POINT_SIZE*2.))-1, -((j+TIME_BAR/(POINT_SIZE)-0.5)/(window0_size.at(1)/(POINT_SIZE*2.))-1));
    }
  }
  glEnd();

  sprintf(now,"Current time: %-12lg",current_time);
  write_string(-0.9, 0.9, now);

  if (color_flag == 'u')
    write_color_bar(max_u, min_u);
  else if (color_flag == 'v')
    write_color_bar(max_v, min_v);
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

  printf("%c: ", key);
  switch (key)
  {
    case 'u':
      color_flag = 'u';
      break;
    case 'v':
      color_flag = 'v';
      break;
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
      make_init();
      pre_field_u = field_u;
      pre_field_v = field_v;
      istep = 0;
      current_time = 0.0;
      printf("initialize\n");
      break;
    case 'c':
      for (int i = 1; i <= width; i++)
      {
        for (int j = 1; j <= height; j++)
        {
          field_u.at(i).at(j) = A;
          field_v.at(i).at(j) = B/A;
        }
      }
      pre_field_u = field_u;
      pre_field_v = field_v;
      istep = 0;
      current_time = 0.0;
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
  active_window = glutGetWindow();

  if (active_window == window_id.at(0))
  {
    x = std::min((int)(x/POINT_SIZE), width);
    y = std::min((int)((y-TIME_BAR)/POINT_SIZE), height);
    x = std::max(0, x);
    y = std::max(0, y);
    field_u.at(x).at(y) = 0;
    pre_field_u.at(x).at(y) = 0;
  }
}

void display1()
{
  double max_u, max_v;
  double min_u, min_v;
  vd du(NUM_SEP, 0.0), dv(NUM_SEP, 0.0);

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  min_u = -1., min_v = -1.;
  max_u = A*9., max_v = B/A*3.;
  glColor3f(0, 0, 0);
  glLineWidth(2);
  glBegin(GL_LINES);
    glVertex2f(-0.8,-1);
    glVertex2f(-0.8,1);
  glEnd();
  write_string(-0.9, 0.9, "v");
  glBegin(GL_LINES);
    glVertex2f(-0.8,-0.2);
    glVertex2f(-0.75,-0.2);
  glEnd();
  write_string(-0.9, -0.2, "4");
  glBegin(GL_LINES);
    glVertex2f(-0.8,0.4);
    glVertex2f(-0.75,0.4);
  glEnd();
  write_string(-0.9, 0.4, "8");
  glBegin(GL_LINES);
    glVertex2f(-1,-0.8);
    glVertex2f(1,-0.8);
  glEnd();
  write_string(0.9, -0.9, "u");
  glBegin(GL_LINES);
    glVertex2f(-0.2,-0.8);
    glVertex2f(-0.2,-0.75);
  glEnd();
  write_string(-0.22, -0.9, "3");
  glBegin(GL_LINES);
    glVertex2f(0.4,-0.8);
    glVertex2f(0.4,-0.75);
  glEnd();
  write_string(0.38, -0.9, "6");
  write_string(-0.9, -0.9, "O");

  for (int i = 0; i < NUM_SEP; i++)
  {
    du.at(i) = (B+1)/(max_u/NUM_SEP*(i+1)) - A/((max_u/NUM_SEP*(i+1))*(max_u/NUM_SEP*(i+1)));
    dv.at(i) = B/(max_u/NUM_SEP*(i+1));
  }
  glColor3f(0.5, 1, 0.5);
  for (int i = 0; i < NUM_SEP-1; i++)
  {
    glBegin(GL_LINES);
      glVertex2f(-0.8+(i+1.)/NUM_SEP*1.8,-0.8+du.at(i)/max_v*1.8);
      glVertex2f(-0.8+(i+2.)/NUM_SEP*1.8,-0.8+du.at(i+1)/max_v*1.8);
    glEnd();
  }
  glColor3f(1, 0.5, 1);
  for (int i = 0; i < NUM_SEP-1; i++)
  {
    glBegin(GL_LINES);
    glVertex2f(-0.8+(i+1.)/NUM_SEP*1.8,-0.8+dv.at(i)/max_v*1.8);
    glVertex2f(-0.8+(i+2.)/NUM_SEP*1.8,-0.8+dv.at(i+1)/max_v*1.8);
    glEnd();
  }

  glPointSize(1);
  glColor3f(0, 0, 0);
  glBegin(GL_POINTS);
  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
      glVertex2f(-0.8+field_u.at(i).at(j)/max_u*1.8, -0.8+field_v.at(i).at(j)/max_v*1.8);
  }
  glEnd();
  glutSwapBuffers();
}

void write_string(double x, double y, char str[])
{
  char *tmp;

  glColor3f(0, 0, 0);
  glRasterPos2f(x, y);
  tmp = str;
  while (*tmp)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *tmp++);
  }
}
void write_color_bar(double max_p, double min_p)
{
  double r, g, b;
  double now_p;
  char str_min[256], str_max[256], str_mode[256];

  glLineWidth(LINE_WIDTH);
  glBegin(GL_LINES);
  for (int i = 0; i < 100; i++)
  {
    now_p = max_p + (min_p-max_p)*i/100.;
    r = std::min(1., std::fabs(2*(now_p-min_p)/(max_p-min_p) - 0.5));
    g = std::sin(PI*(now_p-min_p)/(max_p-min_p));
    b = std::cos(PI/2.*(now_p-min_p)/(max_p-min_p));
    glColor3f(r, g, b);
    glVertex2f(1, -((i*height/100.+TIME_BAR/(POINT_SIZE)+0.5)/(window0_size.at(1)/(POINT_SIZE*2.))-1));
    glVertex2f((window0_size.at(0)-COLOR_BAR_WIDTH+2)/(window0_size.at(0)/2.)-1, -((i*height/100.+TIME_BAR/(POINT_SIZE)+0.5)/(window0_size.at(1)/(POINT_SIZE*2.))-1));
  }
  glEnd();
  glFlush();

  sprintf(str_max,"%c = %4.2lg",color_flag, max_p);
  sprintf(str_min,"%c = %4.2lg",color_flag, min_p);
  sprintf(str_mode,"current mode: show %c", color_flag);
  write_string((window0_size.at(0)-COLOR_BAR_WIDTH+2)/(window0_size.at(0)/2.)-0.92, -((0.1*height+TIME_BAR/(POINT_SIZE)+0.5)/(window0_size.at(1)/(POINT_SIZE*2.))-1), str_max);
  write_string((window0_size.at(0)-COLOR_BAR_WIDTH+2)/(window0_size.at(0)/2.)-0.92, -((0.9*height+TIME_BAR/(POINT_SIZE)+0.5)/(window0_size.at(1)/(POINT_SIZE*2.))-1), str_min);
  write_string(0.2, 0.9, str_mode);
}
/* ++++++++++ openGL ++++++++++ */
