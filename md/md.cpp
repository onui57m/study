/**
 *
 * numerically integrating by the leap frog method
 * only LJ potential is considered
 * (U(r)=4*E_LJ*((S_LJ/r)^-12-(S_LJ/r)^-6))
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-06T01:15:23+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-10T13:02:30+09:00
 */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <random>

#define E_LJ 1.
#define S_LJ 1.
#define CUTOFF_LJ 3.
#define MASS 1.
#define PI 3.141592653589793238462

typedef std::vector<double> vd;
typedef std::vector<int> vi;
typedef std::vector<vi> vvi;
typedef std::map<std::string, std::string> map_ss;

void read_param(
  int &total_step, int &log_step, double &dt,
  double &rlist, int &book_interval,
  double &ref_kBT, double &ref_p,
  int &flag_gen_vel, int &seed,
  map_ss &control_param
);
void read_init(
  int &par_num, double &init_kBT,
  double &box_x, double &box_y, double &box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
);
void gen_vel(int par_num, int seed, double init_kBT, vd &vel_x, vd &vel_y, vd &vel_z);
vvi get_verlet_list(
  int par_num, double rlist,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z
);
void calc_force(
  int par_num, map_ss &control_param, vvi &verlet_list,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &force_x, vd &force_y, vd &force_z,
  vd &energy
);
void calc_energy(int par_num, vd &energy, vd &vel_x, vd &vel_y, vd &vel_z);
void calc_dynamics(
  int par_num, map_ss &control_param, vvi &verlet_list,
  double dt, double dt_v,
  double &box_x, double &box_y, double &box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z,
  vd &force_x, vd &force_y, vd &force_z,
  vd &energy
);
void write_info(int now_step, double dt, vd &energy);
void write_cood(
  int par_num, double now_kBT,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
);
void write_traj(
  int par_num, int now_step, double dt,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z
);
void calc_stat();
void write_stat();

void re_pos(double box_x, double box_y, double box_z, double &rx, double &ry, double &rz);

int main()
{
  int par_num, total_step, log_step, now_step, book_interval, flag_gen_vel, seed;
  double box_x, box_y, box_z, dt, dt_v, rlist, init_kBT, ref_kBT, now_kBT, ref_p;
  map_ss control_param;
  vvi verlet_list;
  vd pos_x, pos_y, pos_z;
  vd vel_x, vel_y, vel_z;
  std::ofstream ofs;


  read_param(total_step, log_step, dt, rlist, book_interval, ref_kBT, ref_p, flag_gen_vel, seed, control_param);
  read_init(par_num, init_kBT, box_x, box_y, box_z, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z);
  if (flag_gen_vel)
    gen_vel(par_num, seed, init_kBT, vel_x, vel_y, vel_z);
  dt_v = dt/MASS;

  ofs.open("traj.dat");
  ofs.close();
  ofs.open("info.dat");
  ofs.close();

  vd force_x(par_num, 0.0), force_y(par_num, 0.0), force_z(par_num, 0.0);
  vd energy(3, 0.0); /* total kinetic potential */

  verlet_list = get_verlet_list(par_num, rlist, box_x, box_y, box_z, pos_x, pos_y, pos_z);
  calc_force(par_num, control_param, verlet_list, box_x, box_y, box_z, pos_x, pos_y, pos_z, force_x, force_y, force_z, energy);
  calc_energy(par_num, energy, vel_x, vel_y, vel_z);
  write_info(0, dt, energy);
  write_traj(par_num, 0, dt, box_x, box_y, box_z, pos_x, pos_y, pos_z);
  for (now_step = 1; now_step <= total_step; now_step++)
  {
    calc_dynamics(
      par_num, control_param, verlet_list,
      dt, dt_v,
      box_x, box_y, box_z,
      pos_x, pos_y, pos_z,
      vel_x, vel_y, vel_z,
      force_x, force_y, force_z,
      energy
    );
    if (now_step % book_interval)
      verlet_list = get_verlet_list(par_num, rlist, box_x, box_y, box_z, pos_x, pos_y, pos_z);
    if (now_step % log_step == 0)
    {
      calc_energy(par_num, energy, vel_x, vel_y, vel_z);

      write_info(now_step, dt, energy);
      write_traj(par_num, now_step, dt, box_x, box_y, box_z, pos_x, pos_y, pos_z);
      write_cood(par_num, now_kBT, box_x, box_y, box_z, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z);
    }
  }
  // calc_stat();
  // write_stat();
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
void read_init(
  int &par_num, double &init_kBT,
  double &box_x, double &box_y, double &box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
)
{
  vd tmp(3,0.0);
  std::ifstream finit;

  finit.open("init.dat");
  if (!finit)
  {
    std::cout << "Cannot open init.dat.\n";
    exit(1);
  }
  finit >> par_num >> init_kBT;
  finit >> box_x >> box_y >> box_z;
  for (int i = 0; i < par_num; i++)
  {
    finit >> tmp.at(0) >> tmp.at(1) >> tmp.at(2);
    pos_x.push_back(tmp.at(0));
    pos_y.push_back(tmp.at(1));
    pos_z.push_back(tmp.at(2));
  }
  for (int i = 0; i < par_num; i++)
  {
    finit >> tmp.at(0) >> tmp.at(1) >> tmp.at(2);
    vel_x.push_back(tmp.at(0));
    vel_y.push_back(tmp.at(1));
    vel_z.push_back(tmp.at(2));
  }
  finit.close();
}
void gen_vel(int par_num, int seed, double init_kBT, vd &vel_x, vd &vel_y, vd &vel_z)
{
  double en_tot, ratio;
  vd cov(3,0.0);
  std::mt19937 mt(seed);
  std::uniform_real_distribution<double> dist_x(-1., 1.);
  std::uniform_real_distribution<double> dist_y(-1., 1.);
  std::uniform_real_distribution<double> dist_z(-1., 1.);

  for (int i = 0; i < par_num; i++)
  {
    vel_x.at(i) = dist_x(mt);
    vel_y.at(i) = dist_y(mt);
    vel_z.at(i) = dist_z(mt);
  }

  en_tot = 0.0;
  for (int i = 0; i < par_num; i++)
  {
    cov.at(0) += vel_x.at(i);
    cov.at(1) += vel_y.at(i);
    cov.at(2) += vel_z.at(i);
  }
  for (int i = 0; i < 3; i++)
    cov.at(i) /= par_num;
  for (int i = 0; i < par_num; i++)
  {
    vel_x.at(i) -= cov.at(0);
    vel_y.at(i) -= cov.at(1);
    vel_z.at(i) -= cov.at(2);
    en_tot += vel_x.at(i)*vel_x.at(i) + vel_y.at(i)*vel_y.at(i) + vel_z.at(i)*vel_z.at(i);
  }
  ratio = 1.5*par_num*init_kBT/en_tot;
  ratio = std::sqrt(ratio);
  for (int i = 0; i < par_num; i++)
  {
    vel_x.at(i) *= ratio;
    vel_y.at(i) *= ratio;
    vel_z.at(i) *= ratio;
  }
}
vvi get_verlet_list(
  int par_num, double rlist,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z
)
{
  vd rij(3,0.0);
  double dist;
  vvi verlet_list(par_num);

  for (int i = 0; i < par_num-1; i++)
  {
    for (int j = i+1; j < par_num; j++)
    {
      rij.at(0) = pos_x.at(j) - pos_x.at(i);
      rij.at(1) = pos_y.at(j) - pos_y.at(i);
      rij.at(2) = pos_z.at(j) - pos_z.at(i);
      re_pos(box_x, box_y, box_z, rij.at(0), rij.at(1), rij.at(2));
      dist = rij.at(0)*rij.at(0)+rij.at(1)*rij.at(1)+rij.at(2)*rij.at(2);
      dist = std::sqrt(dist);
      if (dist < rlist)
        verlet_list.at(i).push_back(j);
    }
  }
  return verlet_list;
}
void calc_energy(int par_num, vd &energy, vd &vel_x, vd &vel_y, vd &vel_z)
{
  energy.at(1) = 0.0;

  for (int i = 0; i < par_num; i++)
    energy.at(1) += vel_x.at(i)*vel_x.at(i) + vel_y.at(i)*vel_y.at(i) + vel_z.at(i)*vel_z.at(i);
  energy.at(1) /= 2.0*MASS;

  energy.at(0) = energy.at(1) + energy.at(2);
}
void calc_force(
  int par_num, map_ss &control_param, vvi &verlet_list,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &force_x, vd &force_y, vd &force_z,
  vd &energy
)
{
  vd rij(3,0.0);
  double dist2, dist6, force;

  for (int i = 0; i < par_num; i++)
  {
    force_x.at(i) = 0.0;
    force_y.at(i) = 0.0;
    force_z.at(i) = 0.0;
  }
  energy.at(2) = 0.0;

  for (int i = 0; i < par_num-1; i++)
  {
    if (control_param.at("USE_VERLET_LIST") == "YES")
    {
      for (auto j : verlet_list.at(i))
      {
        rij.at(0) = pos_x.at(j) - pos_x.at(i);
        rij.at(1) = pos_y.at(j) - pos_y.at(i);
        rij.at(2) = pos_z.at(j) - pos_z.at(i);
        re_pos(box_x, box_y, box_z, rij.at(0), rij.at(1), rij.at(2));
        dist2 = rij.at(0)*rij.at(0)+rij.at(1)*rij.at(1)+rij.at(2)*rij.at(2);
        dist6 = (dist2/(S_LJ*S_LJ))*(dist2/(S_LJ*S_LJ))*(dist2/(S_LJ*S_LJ));
        if (dist2 < CUTOFF_LJ*CUTOFF_LJ)
        {
          energy.at(2) += 4*E_LJ*(1-dist6)/(dist6*dist6);
          force = E_LJ*(48.-24.*dist6)/(dist6*dist6*dist2);
          force_x.at(i) -= force*rij.at(0);
          force_y.at(i) -= force*rij.at(1);
          force_z.at(i) -= force*rij.at(2);
          force_x.at(j) += force*rij.at(0);
          force_y.at(j) += force*rij.at(1);
          force_z.at(j) += force*rij.at(2);
        }
      }
    }
    else
    {
      for (int j = i+1; j < par_num; j++)
      {
        rij.at(0) = pos_x.at(j) - pos_x.at(i);
        rij.at(1) = pos_y.at(j) - pos_y.at(i);
        rij.at(2) = pos_z.at(j) - pos_z.at(i);
        re_pos(box_x, box_y, box_z, rij.at(0), rij.at(1), rij.at(2));
        dist2 = rij.at(0)*rij.at(0)+rij.at(1)*rij.at(1)+rij.at(2)*rij.at(2);
        dist6 = (dist2/(S_LJ*S_LJ))*(dist2/(S_LJ*S_LJ))*(dist2/(S_LJ*S_LJ));
        if (dist2 < CUTOFF_LJ*CUTOFF_LJ)
        {
          energy.at(2) += 4*E_LJ*(1-dist6)/(dist6*dist6);
          force = E_LJ*(48.-24.*dist6)/(dist6*dist6*dist2);
          force_x.at(i) -= force*rij.at(0);
          force_y.at(i) -= force*rij.at(1);
          force_z.at(i) -= force*rij.at(2);
          force_x.at(j) += force*rij.at(0);
          force_y.at(j) += force*rij.at(1);
          force_z.at(j) += force*rij.at(2);
        }
      }
    }
  }
}
void calc_dynamics(
  int par_num, map_ss &control_param, vvi &verlet_list,
  double dt, double dt_v,
  double &box_x, double &box_y, double &box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z,
  vd &force_x, vd &force_y, vd &force_z,
  vd &energy
)
{
  for (int i = 0; i < par_num; i++)
  {
    pos_x.at(i) += vel_x.at(i) * dt;
    pos_y.at(i) += vel_y.at(i) * dt;
    pos_z.at(i) += vel_z.at(i) * dt;
  }
  calc_force(par_num, control_param, verlet_list, box_x, box_y, box_z, pos_x, pos_y, pos_z, force_x, force_y, force_z, energy);
  for (int i = 0; i < par_num; i++)
  {
    vel_x.at(i) += force_x.at(i) * dt_v;
    vel_y.at(i) += force_y.at(i) * dt_v;
    vel_z.at(i) += force_z.at(i) * dt_v;
  }
}
void write_info(int now_step, double dt, vd &energy)
{
  std::ofstream finfo;

  finfo.open("info.dat", std::ios::app);
  finfo << std::fixed << std::setprecision(3);
  finfo << std::right << std::setw(10) << now_step*dt << " ";
  finfo << std::right << std::setw(10) << energy.at(0) << " ";
  finfo << std::right << std::setw(10) << energy.at(1) << " ";
  finfo << std::right << std::setw(10) << energy.at(2) << "\n";
  finfo.close();
}
void write_cood(
  int par_num, double now_kBT,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
)
{
  std::ofstream ofs;

  ofs.open("cood.dat");
  if (!ofs)
  {
    std::cout << "Cannot open cood.dat.\n";
    exit(1);
  }
  ofs << std::fixed << std::setprecision(6);
  ofs << std::right << std::setw(12) << par_num << " ";
  ofs << std::right << std::setw(12) << now_kBT << "\n";
  ofs << std::right << std::setw(12) << box_x << " ";
  ofs << std::right << std::setw(12) << box_y << " ";
  ofs << std::right << std::setw(12) << box_z << "\n";
  for (int i = 0; i < par_num; i++)
  {
    ofs << std::right << std::setw(12) << pos_x.at(i) << " ";
    ofs << std::right << std::setw(12) << pos_y.at(i) << " ";
    ofs << std::right << std::setw(12) << pos_z.at(i) << "\n";
  }
  for (int i = 0; i < par_num; i++)
  {
    ofs << std::right << std::setw(12) << vel_x.at(i) << " ";
    ofs << std::right << std::setw(12) << vel_y.at(i) << " ";
    ofs << std::right << std::setw(12) << vel_z.at(i) << "\n";
  }
  ofs.close();
}
void write_traj(
  int par_num, int now_step, double dt,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z
)
{
  std::ofstream ofs;

  ofs.open("traj.dat", std::ios::app);
  if (!ofs)
  {
    std::cout << "Cannot open traj.dat.\n";
    exit(1);
  }
  ofs << std::fixed << std::setprecision(6);
  ofs << std::right << std::setw(12) << now_step*dt << "\n";
  ofs << std::right << std::setw(12) << box_x << " ";
  ofs << std::right << std::setw(12) << box_y << " ";
  ofs << std::right << std::setw(12) << box_z << "\n";
  for (int i = 0; i < par_num; i++)
  {
    ofs << std::right << std::setw(12) << pos_x.at(i) << " ";
    ofs << std::right << std::setw(12) << pos_y.at(i) << " ";
    ofs << std::right << std::setw(12) << pos_z.at(i) << "\n";
  }
  ofs.close();
}

/* ++++++++++ sub functions ++++++++++ */
void re_pos(double box_x, double box_y, double box_z, double &rx, double &ry, double &rz)
{
  if (rx > box_x/2.) rx -= box_x;
  else if (rx < box_x/2.) rx += box_x;
  if (ry > box_y/2.) ry -= box_y;
  else if (ry < box_y/2.) ry += box_y;
  if (rz > box_z/2.) rz -= box_z;
  else if (rz < box_z/2.) rz += box_z;
}
