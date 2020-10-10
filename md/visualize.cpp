/**
 *
 * use OpenGL to visualize
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-07T00:53:13+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-10T18:33:48+09:00
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

#include <GL/gl.h>
#include <GL/glut.h>

#define POINT_SIZE 5.
#define WIDTH 500.
#define HEIGHT 500.
#define PI 3.141592653589793238462
#define TIME_LINE_BYTE 13
#define ONE_LINE_BYTE 39

typedef std::vector<int> vi;
typedef std::vector<double> vd;
typedef std::vector<vd> vvd;
typedef std::map<std::string, std::string> map_ss;

int par_num, total_step, log_step, book_interval, flag_gen_vel, seed;
double box_x, box_y, box_z, dt, current_time, rlist, ref_kBT, ref_p;
map_ss control_param;
vd pos_x, pos_y, pos_z;

int active_window;
vi window_id(2);
double camera_r, camera_t, camera_phi, camera_psi, z_near, z_far;
vd e_camera(3), n_camera(3), h_camera(3), camera_up(3), center(3);
int nojump_flag, line_flag, dim_flag, time_flag, view_mode;
char move_flag;
int file_base_pos;
std::ifstream::off_type traj_off;
std::ifstream::pos_type traj_pos;
std::ifstream ftraj;

void read_param(
  int &total_step, int &log_step, double &dt,
  double &rlist, int &book_interval,
  double &ref_kBT, double &ref_p,
  int &flag_gen_vel, int &seed,
  map_ss &control_param
);
void read_cood();
int get_traj_off(int offset);

void initialize();
void display0();
void display1();
void timer(int a);
void keyboard(unsigned char key, int a, int b);
void spec_keyboard(int key, int a, int b);
void mouse(int button, int state, int x, int y);

vd get_e_camera(double camera_t, double camera_phi);
vd get_n_camera(double camera_t, double camera_phi);
vd get_h_camera(vd &e_camera, vd &n_camera);
vd rotate(vd &axis, vd &rotated, double theta);
void write_string(double x, double y, double z, std::string str);

int main(int argc, char *argv[])
{
  std::ifstream ifs;

  read_param(total_step, log_step, dt, rlist, book_interval, ref_kBT, ref_p, flag_gen_vel, seed, control_param);
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
  glutSpecialFunc(spec_keyboard);
  glutMouseFunc(mouse);

  glutInitWindowPosition(800, 100);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  window_id.at(1) = glutCreateWindow(title1);
  glutDisplayFunc(display1);

  initialize();
  glutTimerFunc(100, timer, 0);
  glutMainLoop();
  /* ++++++++++ OpenGL ++++++++++ */
}

/* ++++++++++ functions ++++++++++ */
void read_param(
  int &total_step, int &log_step, double &dt,
  double &rlist, int &book_interval,
  double &ref_kBT, double &ref_p,
  int &flag_gen_vel, int &seed,
  map_ss &control_param
)
{
  /*
    a list of parameter name

    TOTAL_STEP, LOG_STEP, DT
    USE_VERLET_LIST, TEMPERATURE_COUPLING, PRESSURE_COUPLING
    RLIST, BOOK_INTERVAL
    REF_KBT
    REF_P
    FLAG_GEN_VEL, SEED
  */
  std::ifstream fparam;
  std::string line, param_name, param_value;

  fparam.open("param.dat");
  if (!fparam)
  {
    std::cout << "Cannot open param.dat.\n";
    exit(1);
  }
  while (std::getline(fparam,line))
  {
    if (line.size() == 0 || line.at(0) == '#')
      continue;
    std::stringstream ss{line};
    ss >> param_name;
    if(param_name == "TOTAL_STEP")
      ss >> total_step;
    else if(param_name == "LOG_STEP")
      ss >> log_step;
    else if (param_name == "DT")
      ss >> dt;
    else if (param_name == "USE_VERLET_LIST" ||
             param_name == "TEMPERATURE_COUPLING" ||
             param_name == "PRESSURE_COUPLING")
    {
      ss >> param_value;
      control_param[param_name] = param_value;
    }
    else if (param_name == "RLIST")
      ss >> rlist;
    else if (param_name == "BOOK_INTERVAL")
      ss >> book_interval;
    else if (param_name == "REF_KBT")
      ss >> ref_kBT;
    else if (param_name == "REF_P")
      ss >> ref_p;
    else if (param_name == "FLAG_GEN_VEL")
      ss >> flag_gen_vel;
    else if (param_name == "SEED")
      ss >> seed;
    else
    {
      std::cout << "Unknown parameter: " << param_name << "\n";
      exit(1);
    }
  }
  fparam.close();
}
void read_cood()
{
  if (move_flag == 's')
  {
    if (traj_off + ftraj.tellg() > (TIME_LINE_BYTE+ONE_LINE_BYTE*(par_num+1))*(total_step/log_step))
    {
      traj_off = (TIME_LINE_BYTE+ONE_LINE_BYTE*(par_num+1))*(total_step/log_step) - ftraj.tellg();
      move_flag = 'p';
    }
    else if (traj_off + ftraj.tellg() < 0)
    {
      traj_off = -ftraj.tellg();
      move_flag = 'p';
    }
  }
  if (file_base_pos)
    ftraj.seekg(traj_off,std::ios_base::cur);
  else
    ftraj.seekg(traj_off,std::ios_base::beg);
  traj_pos = ftraj.tellg();
  // std::cout << "move_flag = " << move_flag << ", traj_off = " << traj_off << ", traj_pos = " << traj_pos << ", ftraj.eof() = " << ftraj.eof() << "\n";
  ftraj >> current_time;
  ftraj >> box_x >> box_y >> box_z;
  for (int i = 0; i < par_num; i++)
  {
    ftraj >> pos_x.at(i) >> pos_y.at(i) >> pos_z.at(i);
  }
  if (move_flag == 'p')
  {
    traj_off = 0;
    file_base_pos = 1;
    ftraj.seekg(traj_pos);
  }
  else
    ftraj.seekg(1,std::ios_base::cur);
  if (current_time == total_step*dt)
  {
    ftraj.clear();
    ftraj.seekg(traj_pos);
    traj_off = 0;
    file_base_pos = 1;
    move_flag = 'p';
  }
}
int get_traj_off(int offset)
{
  int now_step, rest;
  if (file_base_pos != 0)
    now_step = current_time/dt;
  else
    now_step = 0;
  if (move_flag != 'p')
  {
    now_step++;
    if (offset == 5)
      offset = 3;
    if (offset == -1)
      offset = -3;
  }

  if (offset > 0)
  {
    rest = (total_step - now_step)/log_step;
    offset = std::min(offset, rest);
  }
  else if (offset < 0)
  {
    rest = now_step/log_step;
    offset = std::max(offset, -rest);
  }
  return (TIME_LINE_BYTE+ONE_LINE_BYTE*(par_num+1))*offset;
}

/* ++++++++++ OpenGL ++++++++++ */
void initialize()
{
  move_flag = 'p';
  nojump_flag = -1;
  line_flag = -1;
  dim_flag = -1;
  time_flag = -1;
  view_mode = 0;
  traj_off = 0;
  file_base_pos = 0;
  ftraj.seekg(0,std::ios_base::beg);
  traj_pos = ftraj.tellg();
  glutSetWindow(window_id.at(0));
  active_window = window_id.at(0);

  camera_r = 10.;
  camera_t = 0.;
  camera_phi = 0.;
  camera_psi = 0.;
  center.at(0) = 0.;
  center.at(1) = 0.;
  center.at(2) = 0.;
  e_camera = get_e_camera(camera_t, camera_phi);
  n_camera = get_n_camera(camera_t, camera_phi);
  h_camera = get_h_camera(e_camera, n_camera);
  camera_up = rotate(e_camera, n_camera, camera_psi);
  z_near = 1.0;
  z_far = 100000.0;
  glLoadIdentity();
  gluPerspective(20.0, WIDTH/HEIGHT, z_near, z_far);
  gluLookAt(
    camera_r*e_camera.at(0)+center.at(0), camera_r*e_camera.at(1)+center.at(1), camera_r*e_camera.at(2)+center.at(2),
    center.at(0), center.at(1), center.at(2),
    camera_up.at(0), camera_up.at(1), camera_up.at(2)
  );
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST);
}
void display0()
{
  char now[256];
  GLdouble particles[par_num][3];

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (view_mode == 0)
  {
    z_near = 1.0;
    z_far = 100000.0;
  }
  else
  {
    z_near = camera_r - 1.;
    z_far = camera_r + 1.;
  }
  glLoadIdentity();
  gluPerspective(20.0, WIDTH/HEIGHT, z_near, z_far);
  gluLookAt(
    camera_r*e_camera.at(0)+center.at(0), camera_r*e_camera.at(1)+center.at(1), camera_r*e_camera.at(2)+center.at(2),
    center.at(0), center.at(1), center.at(2),
    camera_up.at(0), camera_up.at(1), camera_up.at(2)
  );

  read_cood();

  glPointSize(5);
  glColor3f(1, 0, 0);
  for (int i = 0; i < par_num; i++)
  {
    particles[i][0] = pos_x.at(i);
    particles[i][1] = pos_y.at(i);
    particles[i][2] = pos_z.at(i);
    if(nojump_flag > 0)
    {
      while (particles[i][0] < -box_x/2.)
        particles[i][0] += box_x;
      while (particles[i][0] > box_x/2.)
        particles[i][0] -= box_x;
      while (particles[i][1] < -box_y/2.)
        particles[i][1] += box_y;
      while (particles[i][1] > box_y/2.)
        particles[i][1] -= box_y;
      while (particles[i][2] < -box_z/2.)
        particles[i][2] += box_z;
      while (particles[i][2] > box_z/2.)
        particles[i][2] -= box_z;
    }
  }
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, particles);
  glDrawArrays(GL_POINTS, 0, par_num);
  glDisableClientState(GL_VERTEX_ARRAY);

  if (line_flag > 0)
  {
    glLoadIdentity();
    gluPerspective(20.0, WIDTH/HEIGHT, 1.0, 100000.0);
    gluLookAt(
      camera_r*e_camera.at(0)+center.at(0), camera_r*e_camera.at(1)+center.at(1), camera_r*e_camera.at(2)+center.at(2),
      center.at(0), center.at(1), center.at(2),
      camera_up.at(0), camera_up.at(1), camera_up.at(2)
    );
    glColor3f(0.0, 0.0, 1.0);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
      glVertex3d(-box_x/2., -box_y/2., -box_z/2.);
      glVertex3d(-box_x/2., box_y/2., -box_z/2.);
      glVertex3d(box_x/2., box_y/2., -box_z/2.);
      glVertex3d(box_x/2., -box_y/2., -box_z/2.);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3d(-box_x/2., -box_y/2., box_z/2.);
      glVertex3d(-box_x/2., box_y/2., box_z/2.);
      glVertex3d(box_x/2., box_y/2., box_z/2.);
      glVertex3d(box_x/2., -box_y/2., box_z/2.);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3d(-box_x/2., box_y/2., box_z/2.);
      glVertex3d(-box_x/2., box_y/2., -box_z/2.);
      glVertex3d(box_x/2., box_y/2., -box_z/2.);
      glVertex3d(box_x/2., box_y/2., box_z/2.);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3d(-box_x/2., -box_y/2., box_z/2.);
      glVertex3d(-box_x/2., -box_y/2., -box_z/2.);
      glVertex3d(box_x/2., -box_y/2., -box_z/2.);
      glVertex3d(box_x/2., -box_y/2., box_z/2.);
    glEnd();
  }
  if (dim_flag > 0)
  {
    glLoadIdentity();
    gluPerspective(20.0, WIDTH/HEIGHT, 1.0, 100000.0);
    glTranslated(-1.2,-1.2,0);
    gluLookAt(
      10*e_camera.at(0), 10*e_camera.at(1), 10*e_camera.at(2),
      0.0, 0.0, 0.0,
      camera_up.at(0), camera_up.at(1), camera_up.at(2)
    );
    glLineWidth(5);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
      glVertex3d(0., 0., 0.);
      glVertex3d(0.3, 0., 0.);
    glEnd();
    write_string(0.4, 0., 0., "x");
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
      glVertex3d(0., 0., 0.);
      glVertex3d(0., 0.3, 0.);
    glEnd();
    write_string(0., 0.4, 0., "y");
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
      glVertex3d(0., 0., 0.);
      glVertex3d(0., 0., 0.3);
    glEnd();
    write_string(0., 0., 0.4, "z");
  }
  if (time_flag > 0)
  {
    glLoadIdentity();
    gluPerspective(20.0, WIDTH/HEIGHT, 1.0, 100000.0);
    char now[256];
    sprintf(now,"Current time: %-12lg",current_time);
    std::string now_str(now);
    write_string(-0.15, 0.15, -1., now_str);
  }


  glutSwapBuffers();
}
void display1()
{
  char now[256];

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluPerspective(20.0, WIDTH/HEIGHT, 1.0, 100000.0);
  gluLookAt(
    camera_r*std::sin(PI*camera_t/180.)*std::cos(PI*camera_phi/180.)-center.at(0), camera_r*std::sin(PI*camera_t/180.)*std::sin(PI*camera_phi/180.)-center.at(1), camera_r*std::cos(PI*camera_t/180.)-center.at(2),
    center.at(0), center.at(1), center.at(2),
    camera_up.at(0), camera_up.at(1), camera_up.at(2)
  );

  glPointSize(5);
  glColor3f(1, 0, 0);
  glBegin(GL_POINTS);
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glEnd();


  glutSwapBuffers();
}
void timer(int a)
{
  active_window = glutGetWindow();
  glutSetWindow(window_id.at(0));
  glutPostRedisplay();
  glutSetWindow(window_id.at(1));
  glutPostRedisplay();
  glutSetWindow(active_window);

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
        file_base_pos = 1;
        traj_off = 0;
        move_flag = 's';
        break;
      case 'i':
        file_base_pos = 0;
        traj_off = 0;
        move_flag = 'p';
        break;
      case 'e':
        file_base_pos = 0;
        traj_off = get_traj_off(total_step/log_step);
        move_flag = 'p';
        break;
      case 'a':
        file_base_pos = 1;
        traj_off = get_traj_off(1);
        break;
      case 'A':
        file_base_pos = 1;
        traj_off = get_traj_off(5);
        break;
      case 'b':
        file_base_pos = 1;
        traj_off = get_traj_off(-1);
        break;
      case 'B':
        file_base_pos = 1;
        traj_off = get_traj_off(-5);
        break;
      case 'x':
        camera_t = 90.;
        camera_phi = 0.;
        camera_psi = 0.;
        center.at(0) = 0.;
        center.at(1) = 0.;
        center.at(2) = 0.;
        e_camera = get_e_camera(camera_t, camera_phi);
        n_camera = get_n_camera(camera_t, camera_phi);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'X':
        camera_t = 90.;
        camera_phi = 180.;
        camera_psi = 0.;
        center.at(0) = 0.;
        center.at(1) = 0.;
        center.at(2) = 0.;
        e_camera = get_e_camera(camera_t, camera_phi);
        n_camera = get_n_camera(camera_t, camera_phi);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'y':
        camera_t = 90.;
        camera_phi = 90.;
        camera_psi = 0.;
        center.at(0) = 0.;
        center.at(1) = 0.;
        center.at(2) = 0.;
        e_camera = get_e_camera(camera_t, camera_phi);
        n_camera = get_n_camera(camera_t, camera_phi);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'Y':
        camera_phi = -90.;
        camera_t = 90.;
        camera_psi = 0.;
        center.at(0) = 0.;
        center.at(1) = 0.;
        center.at(2) = 0.;
        e_camera = get_e_camera(camera_t, camera_phi);
        n_camera = get_n_camera(camera_t, camera_phi);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'z':
        camera_phi = 0.;
        camera_t = 0.;
        camera_psi = 0.;
        center.at(0) = 0.;
        center.at(1) = 0.;
        center.at(2) = 0.;
        e_camera = get_e_camera(camera_t, camera_phi);
        n_camera = get_n_camera(camera_t, camera_phi);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'Z':
        camera_phi = 0.;
        camera_t = 180.;
        camera_psi = 0.;
        center.at(0) = 0.;
        center.at(1) = 0.;
        center.at(2) = 0.;
        e_camera = get_e_camera(camera_t, camera_phi);
        n_camera = get_n_camera(camera_t, camera_phi);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'c':
        camera_r -= 1;
        camera_r = std::max(0.0,camera_r);
        break;
      case 'C':
        camera_r -= 5;
        camera_r = std::max(0.0,camera_r);
        break;
      case 'f':
        camera_r += 1;
        break;
      case 'F':
        camera_r += 5;
        break;
      case 'r':
        camera_psi += 5;
        if (camera_psi > 180)
          camera_psi -= 360;
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'R':
        camera_psi -= 5;
        if (camera_psi < -180)
          camera_psi += 360;
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case 'l':
        line_flag *= -1;
        break;
      case 'j':
        nojump_flag *= -1;
        break;
      case 'd':
        dim_flag *= -1;
        break;
      case 't':
        time_flag *= -1;
        break;
      case 'w':
        view_mode++;
        view_mode %= 2;
        break;
      case 'h':
        std::cout << "show help for md visualize\n";
        std::cout << "if you need detail info, please read the source.\n";

        std::cout << "\n=== step action ===\n";
        std::cout << " p: pose\n";
        std::cout << " s: start\n";
        std::cout << " i: move to initial state (and pose)\n";
        std::cout << " e: move to final state (and pose)\n";
        std::cout << " a: (p mode) go ahead one step   / (s mode) accelerate (*2)\n";
        std::cout << " A: (p mode) go ahead five steps / (s mode) accelerate (*4)\n";
        std::cout << " b: (p mode) go back one step    / (s mode) rewind (*-2)\n";
        std::cout << " B: (p mode) go back five steps  / (s mode) rewind (*-4)\n";

        std::cout << "\n=== camera action ===\n";
        std::cout << " x: get orthographic view on x-axis(+) (yz plane)\n";
        std::cout << " X: get orthographic view on x-axis(-) (yz plane)\n";
        std::cout << " y: get orthographic view on y-axis(+) (xz plane)\n";
        std::cout << " Y: get orthographic view on y-axis(-) (xz plane)\n";
        std::cout << " z: get orthographic view on z-axis(+) (xy plane)\n";
        std::cout << " Z: get orthographic view on z-axis(-) (xy plane)\n";
        std::cout << " c: move camera close to center (|r| -= 1)\n";
        std::cout << " C: move camera close to center (|r| -= 5)\n";
        std::cout << " f: move camera far from center (|r| += 1)\n";
        std::cout << " F: move camera far from center (|r| += 5)\n";

        std::cout << "           r: rotate camera around the line of sight (clockwise)\n";
        std::cout << "           R: rotate camera around the line of sight (counterclockwise)\n";
        std::cout << "  left arrow: rotate camera position around the line of sight (clockwise)\n";
        std::cout << " right arrow: rotate camera position around the line of sight (counterclockwise)\n";
        std::cout << "    up arrow: rotate camera position up\n";
        std::cout << "  down arrow: rotate camera position down\n";

        std::cout << "\n=== view action ===\n";
        std::cout << " l: switch a flag whether to show periodic boundary or not\n";
        std::cout << " j: switch a flag whether to show particles in jumped position or not\n";
        std::cout << " d: switch a flag whether to show the coordinate axes or not\n";
        std::cout << " t: switch a flag whether to show current time or not\n";
        std::cout << " w: switch view modes\n";
        std::cout << "    w0 -> default\n";
        std::cout << "    w1 -> Cross section\n";

        std::cout << "\n q: quit the program\n";
        break;
      case 'q':
        ftraj.close();
        printf("quit\n");
        exit(0);
        break;
      default:
        printf("no action\n");
        break;
    }
  }

}
void spec_keyboard(int key, int a, int b)
{
  active_window = glutGetWindow();

  if (active_window == window_id.at(0))
  {
    switch (key)
    {
      case GLUT_KEY_LEFT:
        e_camera = rotate(n_camera, e_camera, 5.);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case GLUT_KEY_RIGHT:
        e_camera = rotate(n_camera, e_camera, -5.);
        h_camera = get_h_camera(e_camera, n_camera);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case GLUT_KEY_UP:
        e_camera = rotate(h_camera, e_camera, 5.);
        n_camera = rotate(h_camera, n_camera, 5.);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      case GLUT_KEY_DOWN:
        e_camera = rotate(h_camera, e_camera, -5.);
        n_camera = rotate(h_camera, n_camera, -5.);
        camera_up = rotate(e_camera, n_camera, camera_psi);
        break;
      default:
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
        file_base_pos = 1;
        traj_off = 0;
        move_flag = 's';
      break;
    default:
      break;
  }
}

/* ++++++++++ sub functions ++++++++++ */
vd get_e_camera(double camera_t, double camera_phi)
{
  vd r(3);

  r.at(0) = std::sin(PI*camera_t/180.)*std::cos(PI*camera_phi/180.);
  r.at(1) = std::sin(PI*camera_t/180.)*std::sin(PI*camera_phi/180.);
  r.at(2) = std::cos(PI*camera_t/180.);
  return r;
}
vd get_n_camera(double camera_t, double camera_phi)
{
  vd r(3);

  r.at(0) = std::sin(PI*(90-camera_t)/180.)*std::cos(PI*(180+camera_phi)/180.);
  r.at(1) = std::sin(PI*(90-camera_t)/180.)*std::sin(PI*(180+camera_phi)/180.);
  r.at(2) = std::cos(PI*(90-camera_t)/180.);
  return r;
}
vd get_h_camera(vd &e_camera, vd &n_camera)
{
  vd r(3);

  r.at(0) = e_camera.at(1)*n_camera.at(2) - e_camera.at(2)*n_camera.at(1);
  r.at(1) = e_camera.at(2)*n_camera.at(0) - e_camera.at(0)*n_camera.at(2);
  r.at(2) = e_camera.at(0)*n_camera.at(1) - e_camera.at(1)*n_camera.at(0);
  return r;
}
vd rotate(vd &axis, vd &rotated, double theta)
{
  vd r(3, 0.0);
  vvd rot(3, vd(3));

  rot.at(0).at(0) = std::cos(PI*theta/180.) + axis.at(0)*axis.at(0)*(1-std::cos(PI*theta/180.));
  rot.at(0).at(1) = axis.at(0)*axis.at(1)*(1-std::cos(PI*theta/180.)) - axis.at(2)*std::sin(PI*theta/180.);
  rot.at(0).at(2) = axis.at(2)*axis.at(0)*(1-std::cos(PI*theta/180.)) + axis.at(1)*std::sin(PI*theta/180.);
  rot.at(1).at(0) = axis.at(0)*axis.at(1)*(1-std::cos(PI*theta/180.)) + axis.at(2)*std::sin(PI*theta/180.);
  rot.at(1).at(1) = std::cos(PI*theta/180.) + axis.at(1)*axis.at(1)*(1-std::cos(PI*theta/180.));
  rot.at(1).at(2) = axis.at(1)*axis.at(2)*(1-std::cos(PI*theta/180.)) - axis.at(0)*std::sin(PI*theta/180.);
  rot.at(2).at(0) = axis.at(2)*axis.at(0)*(1-std::cos(PI*theta/180.)) - axis.at(1)*std::sin(PI*theta/180.);
  rot.at(2).at(1) = axis.at(1)*axis.at(2)*(1-std::cos(PI*theta/180.)) + axis.at(0)*std::sin(PI*theta/180.);
  rot.at(2).at(2) = std::cos(PI*theta/180.) + axis.at(2)*axis.at(2)*(1-std::cos(PI*theta/180.));

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
      r.at(i) += rot.at(i).at(j)*rotated.at(j);
  }
  return r;
}

void write_string(double x, double y, double z, std::string str)
{
  glColor3f(0, 0, 0);
  glRasterPos3d(x, y, z);

  for (int i = 0; i < str.size(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}
