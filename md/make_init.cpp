/**
 *
 * make initial configuration for md
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-06T01:47:30+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-06T06:08:20+09:00
 */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

#define S_LJ 1.
#define PI 3.141592653589793238462

typedef std::vector<double> vd;

int make_init_pos(int par_num, int init_seed, double box_x, double box_y, double box_z, vd &pos_x, vd &pos_y, vd &pos_z);
int gen_vel(int par_num, int init_seed, double kBT, vd &vel_x, vd &vel_y, vd &vel_z);

void re_pos(double box_x, double box_y, double box_z, double &rx, double &ry, double &rz);

int main()
{
  double box_x, box_y, box_z, kBT;
  int par_num;
  int init_seed;
  std::vector<int> errcheck(2, 0);
  std::ifstream ifs;
  std::ofstream ofs;

  ifs.open("init_param.dat");
  if (!ifs)
  {
    std::cout << "Cannot open init_param.dat.\n";
    return 1;
  }
  ifs >> box_x >> box_y >> box_z;
  ifs >> par_num;
  ifs >> init_seed >> kBT;
  ifs.close();

  if (0.8*box_x*box_y*box_z < par_num*4*PI*S_LJ*S_LJ*S_LJ/3.)
  {
    std::cout << "Too small box size!\n";
    std::cout << "0.8*box size: " << 0.8*box_x*box_y*box_z << "\n";
    std::cout << "total volume of particles: " << par_num*4*PI*S_LJ*S_LJ*S_LJ/3. << "\n";
    return 1;
  }
  vd pos_x(par_num, 0), pos_y(par_num, 0), pos_z(par_num, 0);
  vd vel_x(par_num, 0), vel_y(par_num, 0), vel_z(par_num, 0);

  errcheck.at(0) = make_init_pos(par_num, init_seed, box_x, box_y, box_z, pos_x, pos_y, pos_z);
  errcheck.at(1) = gen_vel(par_num, init_seed, kBT, vel_x, vel_y, vel_z);
  if (errcheck.at(0) || errcheck.at(1))
  {
    if (errcheck.at(0))
      std::cout << "Cannot make initial configuration.\n";
    else
      std::cout << "Cannot generate initial velocities.\n";
    return 1;
  }

  ofs.open("init.dat");
  if (!ofs)
  {
    std::cout << "Cannot open init.dat.\n";
    return 1;
  }
  ofs << par_num << " " << kBT << "\n";
  ofs << box_x << " " << box_y << " " << box_z << "\n";
  ofs << std::fixed << std::setprecision(6);
  for (int i = 0; i < par_num; i++)
  {
    ofs << std::right << std::setw(10) << pos_x.at(i) << " ";
    ofs << std::right << std::setw(10) << pos_y.at(i) << " ";
    ofs << std::right << std::setw(10) << pos_z.at(i) << "\n";
  }
  for (int i = 0; i < par_num; i++)
  {
    ofs << std::right << std::setw(10) << vel_x.at(i) << " ";
    ofs << std::right << std::setw(10) << vel_y.at(i) << " ";
    ofs << std::right << std::setw(10) << vel_z.at(i) << "\n";
  }
  ofs.close()
}

/* ++++++++++ functions ++++++++++ */
int make_init_pos(int par_num, int init_seed, double box_x, double box_y, double box_z, vd &pos_x, vd &pos_y, vd &pos_z)
{
  int limit_count, overlap_num;
  vd rij(3,0.0), com(3,0.0);
  double distance, diff;
  std::mt19937 mt(init_seed);
  std::uniform_real_distribution<double> dist_x(-box_x/2., box_x/2.);
  std::uniform_real_distribution<double> dist_y(-box_y/2., box_y/2.);
  std::uniform_real_distribution<double> dist_z(-box_z/2., box_z/2.);

  for (int i = 0; i < par_num; i++)
  {
    pos_x.at(i) = dist_x(mt);
    pos_y.at(i) = dist_y(mt);
    pos_z.at(i) = dist_z(mt);
  }

  limit_count = 100000;
  while (limit_count)
  {
    overlap_num = 0;
    for (int i = 0; i < par_num; i++)
    {
      for (int j = i+1; j < par_num; j++)
      {
        rij.at(0) = pos_x.at(j) - pos_x.at(i);
        rij.at(1) = pos_y.at(j) - pos_y.at(i);
        rij.at(2) = pos_z.at(j) - pos_z.at(i);
        re_pos(box_x, box_y, box_z, rij.at(0), rij.at(1), rij.at(2));
        distance = rij.at(0)*rij.at(0)+rij.at(1)*rij.at(1)+rij.at(2)*rij.at(2);
        distance = std::sqrt(distance);
        if (distance < S_LJ)
        {
          overlap_num++;
          diff = (S_LJ-distance+0.01)/2.;
          pos_x.at(i) -= diff*rij.at(0);
          pos_y.at(i) -= diff*rij.at(1);
          pos_z.at(i) -= diff*rij.at(2);
          pos_x.at(j) += diff*rij.at(0);
          pos_y.at(j) += diff*rij.at(1);
          pos_z.at(j) += diff*rij.at(2);
        }
      }
    }
    if (overlap_num == 0)
      break;
    limit_count--;
  }
  if (overlap_num)
    return 1;
  else
  {
    for (int i = 0; i < par_num; i++)
    {
      re_pos(box_x, box_y, box_z, pos_x.at(i), pos_y.at(i), pos_z.at(i));
      com.at(0) += pos_x.at(i);
      com.at(1) += pos_y.at(i);
      com.at(2) += pos_z.at(i);
    }
    for (int i = 0; i < 3; i++)
      com.at(i) /= par_num;
    for (int i = 0; i < par_num; i++)
    {
      pos_x.at(i) -= com.at(0);
      pos_y.at(i) -= com.at(1);
      pos_z.at(i) -= com.at(2);
    }
  }

  return 0;
}
int gen_vel(int par_num, int init_seed, double kBT, vd &vel_x, vd &vel_y, vd &vel_z)
{
  double en_tot, ratio;
  vd cov(3,0.0);
  std::mt19937 mt(init_seed+1);
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
  ratio = 1.5*par_num*kBT/en_tot;
  ratio = std::sqrt(ratio);
  for (int i = 0; i < par_num; i++)
  {
    vel_x.at(i) *= ratio;
    vel_y.at(i) *= ratio;
    vel_z.at(i) *= ratio;
  }
  return 0;
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
