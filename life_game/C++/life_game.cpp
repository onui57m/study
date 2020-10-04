/**
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-04T03:33:22+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-04T14:21:02+09:00
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <random>

typedef std::vector<int> vi;
typedef std::vector<vi> vvi;

void make_init(vvi &field, int width, int height);
void write_state(vvi &field, int width, int height);
vvi calc_react(vvi &pre_field, int width, int height, int boundary);

int main(int argc, char *argv[])
{
  std::ifstream ifs;
  char file_name[256];
  int width, height, boundary;
  int total_step, log_step;

  sprintf(file_name,"param.dat");
  ifs.open(file_name);
  if (!ifs)
  {
    std::cout << "Cannot open " << file_name << ".\n";
    return 1;
  }
  ifs >> width >> height >> boundary;
  ifs >> total_step >> log_step;
  ifs.close();

  vvi field(width+2, vi(height+2)), pre_field;
  make_init(field, width, height);
  pre_field = field;

  for (int i = 0; i < total_step; i++)
  {
    if (i % log_step == 0)
      write_state(field, width, height);
    field = calc_react(pre_field, width, height, boundary);
    pre_field = field;
  }
  write_state(field, width, height);
}

void make_init(vvi &field, int width, int height)
{
  int init_seed = 1;
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
void write_state(vvi &field, int width, int height)
{
  static int index;
  char file_name[256];
  std::ofstream ofs;

  std::sprintf(file_name,"dat/state_%03d.dat",index);
  ofs.open(file_name);
  for (int i = 1; i <= width; i++)
  {
    for (int j = 1; j <= height; j++)
      ofs << field.at(i).at(j) << " ";
    ofs << "\n";
  }
  ofs.close();
  index++;
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
