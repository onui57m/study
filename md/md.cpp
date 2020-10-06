/**
 *
 * numerically integrating by the leap frog method
 * only LJ potential is considered
 * (U(r)=4*E_LJ*((S_LJ/r)^-12-(S_LJ/r)^-6))
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-06T01:15:23+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-07T04:37:11+09:00
 */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

#define E_LJ 1.
#define S_LJ 1.
#define CUTOFF_LJ 3.
#define MASS 1.
#define PI 3.141592653589793238462

typedef std::vector<double> vd;

void read_param(
  int &total_step, int &log_step, double &dt,
  double &fin_kBT, double &temper_speed,
  int &flag_gen_vel, int &seed
);
void read_init(
  int &par_num, double &init_kBT,
  double &box_x, double &box_y, double &box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
);
void gen_vel(int par_num, int seed, double init_kBT, vd &vel_x, vd &vel_y, vd &vel_z);
void calc_energy(
  int par_num, vd &energy,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
);
void calc_force(
  int par_num,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &force_x, vd &force_y, vd &force_z
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
  int par_num, total_step, log_step, now_step, flag_gen_vel, seed;
  double box_x, box_y, box_z, dt, dt_v, init_kBT, fin_kBT, now_kBT, temper_speed;
  vd pos_x, pos_y, pos_z;
  vd vel_x, vel_y, vel_z;
  std::ofstream ofs;


  read_param(total_step, log_step, dt, fin_kBT, temper_speed, flag_gen_vel, seed);
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

  calc_energy(par_num, energy, box_x, box_y, box_z, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z);
  write_info(0, dt, energy);
  write_traj(par_num, 0, dt, box_x, box_y, box_z, pos_x, pos_y, pos_z);
  for (now_step = 1; now_step <= total_step; now_step++)
  {
    for (int i = 0; i < par_num; i++)
    {
      pos_x.at(i) += vel_x.at(i) * dt;
      pos_y.at(i) += vel_y.at(i) * dt;
      pos_z.at(i) += vel_z.at(i) * dt;
    }
    calc_force(par_num, box_x, box_y, box_z, pos_x, pos_y, pos_z, force_x, force_y, force_z);
    for (int i = 0; i < par_num; i++)
    {
      vel_x.at(i) += force_x.at(i) * dt_v;
      vel_y.at(i) += force_y.at(i) * dt_v;
      vel_z.at(i) += force_z.at(i) * dt_v;
    }

    if (now_step % log_step == 0)
    {
      calc_energy(par_num, energy, box_x, box_y, box_z, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z);

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
  double &fin_kBT, double &temper_speed,
  int &flag_gen_vel, int &seed
)
{
  std::ifstream fparam;

  fparam.open("param.dat");
  if (!fparam)
  {
    std::cout << "Cannot open param.dat.\n";
    exit(1);
  }
  fparam >> total_step >> log_step >> dt;
  fparam >> fin_kBT >> temper_speed;
  fparam >> flag_gen_vel >> seed;
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
void calc_energy(
  int par_num, vd &energy,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &vel_x, vd &vel_y, vd &vel_z
)
{
  vd rij(3,0.0);
  double dist2, dist6;

  for (int i = 0; i < energy.size(); i++)
    energy.at(i) = 0.0;

  for (int i = 0; i < par_num; i++)
    energy.at(1) += vel_x.at(i)*vel_x.at(i) + vel_y.at(i)*vel_y.at(i) + vel_z.at(i)*vel_z.at(i);
  energy.at(1) /= 2.0*MASS;

  for (int i = 0; i < par_num; i++)
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
        energy.at(2) += 4*E_LJ*(1-dist6)/(dist6*dist6);
    }
  }
  energy.at(0) = energy.at(1) + energy.at(2);
}
void calc_force(
  int par_num,
  double box_x, double box_y, double box_z,
  vd &pos_x, vd &pos_y, vd &pos_z,
  vd &force_x, vd &force_y, vd &force_z
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

  for (int i = 0; i < par_num; i++)
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
