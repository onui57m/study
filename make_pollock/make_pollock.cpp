/**
 *
 * making pollocizied image (fractal)
 * It is assumed that ref_image is a 24-bit bitmap
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-12-28T22:08:39+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-12-29T01:52:57+09:00
 */

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <sys/time.h>

#define PI 3.141592653589703238462
#define INFL 9223372036854775807

typedef std::vector<unsigned char> VC;
typedef std::vector<VC> VVC;
typedef std::vector<VVC> VVVC;
typedef std::vector<std::vector<std::set<int> > > VVSI;

struct s_wind
{
  int start_x;
  int start_y;
  double direc;
  double power;
  VC rgb;
};
struct s_BITMAPFILEHEADER
{
  unsigned short bfType; /* 2 byte, always 'BM' */
  unsigned int bfSize; /* 4 byte, filesize [byte] */
  unsigned short bfReserved1; /* 2 byte, always 0 */
  unsigned short bfReserved2; /* 2 byte, always 0 */
  unsigned int bfOffBits; /* 4 byte, the offset size to the image data [byte] */
};
struct s_BITMAPINFOHEADER
{
  unsigned int biSize; /* 4 byte, the size of info header size [byte] */
  int biWidth; /* 4 byte, the number of pixels in the width ot the image [pixel] */
  int biHeight; /* 4 byte, the number of pixels in the height ot the image [pixel] */
  unsigned short biPlanes; /* 2 byte, the number of plane, always 0 */
  unsigned int biBitCount; /* 2 byte, the number of color bits [bit] */
  unsigned int biCompression; /* 4 byte, a form of compression (0,1,2,3) */
  unsigned int biSizeImage; /* 4 byte, the size of the image data [byte] */
  int biXPixPerMeter; /* 4 byte, horizontal resolution [dot/m] */
  int biYPixPerMeter; /* 4 byte, vertical resolution [dot/m] */
  unsigned int biClrUsed; /* 4 byte, the number of palette */
  unsigned int biCirImportant; /* 4 byte, the number of important color */
};
struct s_RGBQUAD
{
  unsigned char rgbBlue; /* palette data for blue, 0-255 */
  unsigned char rgbGreen; /* palette data for green, 0-255 */
  unsigned char rgbRed; /* palette data for red, 0-255 */
  unsigned char rgbReserved; /* always 0 */
};
struct s_bmp
{
  s_BITMAPFILEHEADER file;
  s_BITMAPINFOHEADER info;
  s_RGBQUAD rgb; /* if biBitCount == 24, rgb does not exist. */
  VVVC image;
};

s_wind get_wind(int width, int height, VVC &color_set);
void read_bmp(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory);
void write_bmp(s_bmp &out_image);
void initialize(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &score);
void draw_line(VVVC &image, VVSI &image_memory, s_wind wind);
void k_means_pp(s_bmp &ref_image, int k_cluster, VVC &color_set);
void anneal(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &score);
void change_wind(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &diff);
void blow_wind(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &diff);

int main(int argc, char *argv[])
{
  s_bmp out_image, ref_image;
  std::vector<s_wind> wind_memory;
  int k_cluster;
  long score;
  VVC color_set;
  VVSI image_memory;

  k_cluster = 8;
  if (argc > 1)
    k_cluster = std::atoi(argv[1]);
  read_bmp(out_image, ref_image, image_memory);
  k_means_pp(ref_image, k_cluster, color_set);
  initialize(out_image, ref_image, image_memory, wind_memory, color_set, score);
  anneal(out_image, ref_image, image_memory, wind_memory, color_set, score);
  write_bmp(out_image);
}

s_wind get_wind(int width, int height, VVC &color_set)
{
  static int gen_seed = 3141592;
  s_wind wind;
  std::mt19937 mt(gen_seed);
  std::uniform_real_distribution<double> dist_dir(-PI, PI);
  std::uniform_real_distribution<double> dist_pow(width/20., width/10.);
  std::uniform_int_distribution<int> dist_x(0, height-1);
  std::uniform_int_distribution<int> dist_y(0, width-1);
  std::uniform_int_distribution<int> dist_rgb(0, color_set.size()-1);

  wind.direc = dist_dir(mt);
  wind.power = dist_pow(mt);
  wind.start_x = dist_x(mt);
  wind.start_y = dist_y(mt);
  wind.rgb = color_set.at(dist_rgb(mt));
  gen_seed++;
  return wind;
}
void read_bmp(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory)
{
  char dummy;
  std::ifstream fbmp;
  fbmp.open("ref.bmp", std::ios::binary);
  if (!fbmp)
  {
    std::cerr << "Cannot open a file.\n";
    exit(1);
  }
  fbmp.read((char *) &ref_image.file.bfType, sizeof(unsigned short));
  fbmp.read((char *) &ref_image.file.bfSize, sizeof(unsigned int));
  fbmp.read((char *) &ref_image.file.bfReserved1, sizeof(unsigned short));
  fbmp.read((char *) &ref_image.file.bfReserved2, sizeof(unsigned short));
  fbmp.read((char *) &ref_image.file.bfOffBits, sizeof(unsigned int));
  fbmp.read((char *) &ref_image.info.biSize, sizeof(unsigned int));
  fbmp.read((char *) &ref_image.info.biWidth, sizeof(int));
  fbmp.read((char *) &ref_image.info.biHeight, sizeof(int));
  fbmp.read((char *) &ref_image.info.biPlanes, sizeof(unsigned short));
  fbmp.read((char *) &ref_image.info.biBitCount, sizeof(unsigned short));
  fbmp.read((char *) &ref_image.info.biCompression, sizeof(unsigned int));
  fbmp.read((char *) &ref_image.info.biSizeImage, sizeof(unsigned int));
  fbmp.read((char *) &ref_image.info.biXPixPerMeter, sizeof(int));
  fbmp.read((char *) &ref_image.info.biYPixPerMeter, sizeof(int));
  fbmp.read((char *) &ref_image.info.biClrUsed, sizeof(unsigned int));
  fbmp.read((char *) &ref_image.info.biCirImportant, sizeof(unsigned int));
  if (ref_image.info.biBitCount < 24)
  {
    fbmp.read((char *) &ref_image.rgb.rgbBlue, sizeof(unsigned char));
    fbmp.read((char *) &ref_image.rgb.rgbGreen, sizeof(unsigned char));
    fbmp.read((char *) &ref_image.rgb.rgbRed, sizeof(unsigned char));
    fbmp.read((char *) &ref_image.rgb.rgbReserved, sizeof(unsigned char));
  }

  VVC one_line(ref_image.info.biWidth,VC(3,255));
  std::vector<std::set<int> > blank_memory(ref_image.info.biWidth);
  for (int i = 0; i < ref_image.info.biHeight; i++)
  {
    ref_image.image.push_back(one_line);
    out_image.image.push_back(one_line);
    image_memory.push_back(blank_memory);
    for (int j = 0; j < ref_image.info.biWidth; j++)
    {
      for (int k = 0; k < 3; k++)
        fbmp.read((char *) &ref_image.image.at(i).at(j).at(k), sizeof(char));
    }
    if (ref_image.info.biWidth % 4)
    {
      for (int j = 0; j < 4 - out_image.info.biWidth*3 % 4; j++)
        fbmp.read((char *) &dummy, sizeof(char));
    }
  }
  fbmp.close();
  out_image.file = ref_image.file;
  out_image.info = ref_image.info;
}
void write_bmp(s_bmp &out_image)
{
  char dummy;
  std::ofstream fbmp;
  fbmp.open("fractal.bmp", std::ios::binary);
  if (!fbmp)
  {
    std::cerr << "Cannot open a file.\n";
    exit(1);
  }
  fbmp.write((char *) &out_image.file.bfType, sizeof(unsigned short));
  fbmp.write((char *) &out_image.file.bfSize, sizeof(unsigned int));
  fbmp.write((char *) &out_image.file.bfReserved1, sizeof(unsigned short));
  fbmp.write((char *) &out_image.file.bfReserved2, sizeof(unsigned short));
  fbmp.write((char *) &out_image.file.bfOffBits, sizeof(unsigned int));
  fbmp.write((char *) &out_image.info.biSize, sizeof(unsigned int));
  fbmp.write((char *) &out_image.info.biWidth, sizeof(int));
  fbmp.write((char *) &out_image.info.biHeight, sizeof(int));
  fbmp.write((char *) &out_image.info.biPlanes, sizeof(unsigned short));
  fbmp.write((char *) &out_image.info.biBitCount, sizeof(unsigned short));
  fbmp.write((char *) &out_image.info.biCompression, sizeof(unsigned int));
  fbmp.write((char *) &out_image.info.biSizeImage, sizeof(unsigned int));
  fbmp.write((char *) &out_image.info.biXPixPerMeter, sizeof(int));
  fbmp.write((char *) &out_image.info.biYPixPerMeter, sizeof(int));
  fbmp.write((char *) &out_image.info.biClrUsed, sizeof(unsigned int));
  fbmp.write((char *) &out_image.info.biCirImportant, sizeof(unsigned int));
  if (out_image.info.biBitCount < 24)
  {
    fbmp.write((char *) &out_image.rgb.rgbBlue, sizeof(unsigned char));
    fbmp.write((char *) &out_image.rgb.rgbGreen, sizeof(unsigned char));
    fbmp.write((char *) &out_image.rgb.rgbRed, sizeof(unsigned char));
    fbmp.write((char *) &out_image.rgb.rgbReserved, sizeof(unsigned char));
  }

  dummy = 0;
  for (int i = 0; i < out_image.info.biHeight; i++)
  {
    for (int j = 0; j < out_image.info.biWidth; j++)
    {
      for (int k = 0; k < 3; k++)
        fbmp.write((char *) &out_image.image.at(i).at(j).at(k), sizeof(char));
    }
    if (out_image.info.biWidth % 4)
    {
      for (int j = 0; j < 4 - out_image.info.biWidth*3 % 4; j++)
      {
        fbmp.write((char *) &dummy, sizeof(char));
      }
    }
  }
  fbmp.close();
}
void initialize(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &score)
{
  int wind_num;
  s_wind wind;

  wind_num = 10000;
  for (int i = 0; i < wind_num; i++)
  {
    wind = get_wind(out_image.info.biWidth, out_image.info.biHeight, color_set);
    wind_memory.push_back(wind);
    draw_line(out_image.image, image_memory, wind);
  }
  score = 0;
  for (int i = 0; i < out_image.info.biWidth; i++)
  {
    for (int j = 0; j < out_image.info.biHeight; j++)
    {
      for (int k = 0; k < 3; k++)
        score += (out_image.image.at(i).at(j).at(k) - ref_image.image.at(i).at(j).at(k))*(out_image.image.at(i).at(j).at(k) - ref_image.image.at(i).at(j).at(k));
    }
  }
}
void draw_line(VVVC &image, VVSI &image_memory, s_wind wind)
{
  static int count_wind;
  double norm, dy, step_size;

  dy = std::tan(wind.direc);
  step_size = std::min(std::abs(1./std::cos(wind.direc)),wind.power);
  norm = 1 + dy*dy;
  norm = std::sqrt(norm);
  if (wind.direc > -PI/2 || wind.direc <= PI/2)
  {
    for (int i = 0; norm*i < wind.power ; i++)
    {
      if (wind.start_x+i >= image.size()) break;
      for (int j = std::floor(dy*i-step_size/2.); j <= std::floor(dy*i+step_size/2.); j++)
      {
        if (wind.start_y+j < 0) continue;
        if (wind.start_y+j >= image.at(i).size()) break;
        image.at(wind.start_x+i).at(wind.start_y+j) = wind.rgb;
        image_memory.at(wind.start_x+i).at(wind.start_y+j).insert(count_wind);
      }
    }
  }
  else
  {
    for (int i = 0; norm*i < wind.power ; i++)
    {
      if (wind.start_x-i < 0) break;
      for (int j = std::floor(-dy*i-step_size/2.); j <= std::floor(-dy*i+step_size/2.); j++)
      {
        if (wind.start_y+j < 0) continue;
        if (wind.start_y+j >= image.at(i).size()) break;
        image.at(wind.start_x-i).at(wind.start_y+j) = wind.rgb;
        image_memory.at(wind.start_x-i).at(wind.start_y+j).insert(count_wind);
      }
    }
  }
  count_wind++;
}
void k_means_pp(s_bmp &ref_image, int k_cluster, VVC &color_set)
{
  std::mt19937 mt(653589);
  std::uniform_int_distribution<int> dist_center(0, ref_image.info.biWidth*ref_image.info.biHeight-1);
  std::vector<long> prob(ref_image.info.biWidth*ref_image.info.biHeight);
  std::vector<int> blank;
  std::vector<int> label(ref_image.info.biWidth*ref_image.info.biHeight,0), pre_label(ref_image.info.biWidth*ref_image.info.biHeight,0);
  std::vector<std::vector<int> > clusters(k_cluster), sum_color(k_cluster, std::vector<int>(3,0));
  VVC pre_color_set;
  int init, next, diff, icluster, max_count;
  long long prob_next;
  bool unconvergent;

  init = dist_center(mt);
  color_set.push_back(ref_image.image.at(init/ref_image.info.biWidth).at(init%ref_image.info.biWidth));
  while (color_set.size() < k_cluster)
  {
    for (int i = 0; i < ref_image.info.biWidth*ref_image.info.biHeight; i++)
    {
      prob.at(i) = INFL;
      for (int j = 0; j < color_set.size(); j++)
      {
        diff = 0;
        for (int k = 0; k < 3; k++)
          diff += (int)(color_set.at(j).at(k)-ref_image.image.at(i/ref_image.info.biWidth).at(i%ref_image.info.biWidth).at(k))*(color_set.at(j).at(k)-ref_image.image.at(i/ref_image.info.biWidth).at(i%ref_image.info.biWidth).at(k));
        if (i) prob.at(i) = std::min(prob.at(i), prob.at(i-1)+diff);
        else prob.at(i) = std::min(prob.at(i), (long)diff);
      }
    }
    std::uniform_int_distribution<long long> dist_prob(1, prob.at(prob.size()-1));
    prob_next = dist_prob(mt);
    next = lower_bound(prob.begin(), prob.end(), prob_next) - prob.begin();
    color_set.push_back(ref_image.image.at(next/ref_image.info.biWidth).at(next%ref_image.info.biWidth));
  }
  unconvergent = true;
  max_count = 1000;
  while (unconvergent && max_count)
  {
    unconvergent = false;
    pre_color_set = color_set;
    pre_label = label;
    for (int i = 0; i < k_cluster; i++)
    {
      for (int j = 0; j < 3; j++)
        sum_color.at(i).at(j) = 0;
    }
    for (int i = 0; i < k_cluster; i++) clusters.at(i) = blank;
    for (int i = 0; i < ref_image.info.biWidth*ref_image.info.biHeight; i++)
    {
      prob.at(i) = INFL;
      for (int j = 0; j < k_cluster; j++)
      {
        diff = 0;
        for (int k = 0; k < 3; k++)
          diff += (int)(pre_color_set.at(j).at(k)-ref_image.image.at(i/ref_image.info.biWidth).at(i%ref_image.info.biWidth).at(k))*(pre_color_set.at(j).at(k)-ref_image.image.at(i/ref_image.info.biWidth).at(i%ref_image.info.biWidth).at(k));
        if (prob.at(i) > diff)
        {
          prob.at(i) = diff;
          icluster = j;
          label.at(i) = j;
        }
      }
      clusters.at(icluster).push_back(i);
      for (int j = 0; j < 3; j++)
        sum_color.at(icluster).at(j) += ref_image.image.at(i/ref_image.info.biWidth).at(i%ref_image.info.biWidth).at(j);
    }
    for (int i = 0; i < k_cluster; i++)
    {
      if (clusters.at(i).size() == 0) continue;
      for (int j = 0; j < 3; j++)
        color_set.at(i).at(j) = (unsigned char)(sum_color.at(i).at(j)/clusters.at(i).size());
    }
    for (int i = 0; i < ref_image.info.biWidth*ref_image.info.biHeight; i++)
    {
      if (label.at(i) != pre_label.at(i))
      {
        unconvergent = true;
        break;
      }
    }
    max_count--;
  }
}
void anneal(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &score)
{
  /* now this is not annealing, but hill-climbing method */

  std::mt19937 mt(793238);
  std::uniform_real_distribution<double> dist_prob(0, 1);
  std::uniform_int_distribution<int> dist_que(0, 1);
  int que;
  double init_temp, fin_temp, current_temp;
  struct timeval tmp_time;
  long long init_time, fin_time, current_time;
  long diff, best_score;
  s_bmp best_image, old_image;
  std::vector<s_wind> best_wind_memory, old_wind_memory;
  VVSI best_image_memory, old_image_memory;
  std::ofstream ftemp;

  // init_temp = 500000, fin_temp = 5000;
  gettimeofday(&tmp_time, NULL);
  init_time = tmp_time.tv_sec*1000000 + tmp_time.tv_usec;
  current_time = init_time;
  fin_time = init_time + 1200000000; /* 20 min */
  best_score = score;
  best_image = out_image;
  best_wind_memory = wind_memory;
  best_image_memory = image_memory;
  ftemp.open("temp.dat");

  while (current_time < fin_time)
  {
    gettimeofday(&tmp_time, NULL);
    current_time = tmp_time.tv_sec*1000000 + tmp_time.tv_usec;
    // current_temp = init_temp + (fin_temp - init_temp)*(current_time-init_time)/(fin_time-init_time);

    for (int i = 0; i < 100; i++)
    {
      old_image = out_image;
      old_wind_memory = wind_memory;
      old_image_memory = image_memory;
      que = dist_que(mt);
      if (que)
        change_wind(out_image, ref_image, image_memory, wind_memory, color_set, diff);
      else
        blow_wind(out_image, ref_image, image_memory, wind_memory, color_set, diff);
      if (/*std::exp(-diff/current_temp) < dist_prob(mt)*/diff >= 0)
      {
        out_image = old_image;
        wind_memory = old_wind_memory;
        image_memory = old_image_memory;
      }
      else
      {
        score += diff;
        if (best_score > score)
        {
          best_score = score;
          best_image = out_image;
          best_wind_memory = wind_memory;
          best_image_memory = image_memory;
        }
      }
    }
    ftemp << current_time <</* " " << current_temp <<*/ " " << score << "\n";
  }
  ftemp.close();
  score = best_score;
  out_image = best_image;
  wind_memory = best_wind_memory;
  image_memory = best_image_memory;
}
void change_wind(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &diff)
{
  static int sel_seed = 462643;
  int sel_index;
  double norm, dy, step_size;
  s_wind wind_before, wind_after;
  VC color_before, color_after;
  bool check;
  std::mt19937 mt(sel_seed);
  std::uniform_int_distribution<int> dist_memory(0, wind_memory.size()-1);

  sel_index = dist_memory(mt);
  wind_before = wind_memory.at(sel_index);
  wind_after = get_wind(out_image.info.biWidth, out_image.info.biHeight, color_set);

  diff = 0;

  /* ===== delete wind before ===== */
  dy = std::tan(wind_before.direc);
  step_size = std::min(std::abs(1./std::cos(wind_before.direc)),wind_before.power);
  norm = 1 + dy*dy;
  norm = std::sqrt(norm);
  if (wind_before.direc > -PI/2 || wind_before.direc <= PI/2)
  {
    for (int i = 0; norm*i < wind_before.power ; i++)
    {
      if (wind_before.start_x+i >= out_image.image.size()) break;
      for (int j = std::floor(dy*i-step_size/2.); j <= std::floor(dy*i+step_size/2.); j++)
      {
        if (wind_before.start_y+j < 0) continue;
        if (wind_before.start_y+j >= out_image.image.at(i).size()) break;
        image_memory.at(wind_before.start_x+i).at(wind_before.start_y+j).erase(sel_index);
        color_before = out_image.image.at(wind_before.start_x+i).at(wind_before.start_y+j);
        if (image_memory.at(wind_before.start_x+i).at(wind_before.start_y+j).size())
          color_after = wind_memory.at( *image_memory.at(wind_before.start_x+i).at(wind_before.start_y+j).rbegin() ).rgb;
        else
          color_after = VC{255, 255, 255};
        check = 0;
        for (int k = 0; k < 3; k++)
        {
          if (color_before.at(k) != color_after.at(k))
          {
            check = 1;
            break;
          }
        }
        if (check)
        {
          out_image.image.at(wind_before.start_x+i).at(wind_before.start_y+j) = color_after;
          for (int k = 0; k < 3; k++)
          {
            diff -= (long)(color_before.at(k) - ref_image.image.at(wind_before.start_x+i).at(wind_before.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind_before.start_x+i).at(wind_before.start_y+j).at(k));
            diff += (long)(color_after.at(k) - ref_image.image.at(wind_before.start_x+i).at(wind_before.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind_before.start_x+i).at(wind_before.start_y+j).at(k));
          }
        }
      }
    }
  }
  else
  {
    for (int i = 0; norm*i < wind_before.power ; i++)
    {
      if (wind_before.start_x-i < 0) break;
      for (int j = std::floor(-dy*i-step_size/2.); j <= std::floor(-dy*i+step_size/2.); j++)
      {
        if (wind_before.start_y+j < 0) continue;
        if (wind_before.start_y+j >= out_image.image.at(i).size()) break;
        image_memory.at(wind_before.start_x-i).at(wind_before.start_y+j).erase(sel_index);
        color_before = out_image.image.at(wind_before.start_x-i).at(wind_before.start_y+j);
        color_after = wind_memory.at( *image_memory.at(wind_before.start_x-i).at(wind_before.start_y+j).rbegin() ).rgb;
        check = 0;
        for (int k = 0; k < 3; k++)
        {
          if (color_before.at(k) != color_after.at(k))
          {
            check = 1;
            break;
          }
        }
        if (check)
        {
          out_image.image.at(wind_before.start_x-i).at(wind_before.start_y+j) = color_after;
          for (int k = 0; k < 3; k++)
          {
            diff -= (long)(color_before.at(k) - ref_image.image.at(wind_before.start_x-i).at(wind_before.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind_before.start_x-i).at(wind_before.start_y+j).at(k));
            diff += (long)(color_after.at(k) - ref_image.image.at(wind_before.start_x-i).at(wind_before.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind_before.start_x-i).at(wind_before.start_y+j).at(k));
          }
        }
      }
    }
  }

  /* ===== insert wind after ===== */
  wind_memory.at(sel_index) = wind_after;
  dy = std::tan(wind_after.direc);
  step_size = std::min(std::abs(1./std::cos(wind_after.direc)),wind_after.power);
  norm = 1 + dy*dy;
  norm = std::sqrt(norm);
  if (wind_after.direc > -PI/2 || wind_after.direc <= PI/2)
  {
    for (int i = 0; norm*i < wind_after.power ; i++)
    {
      if (wind_after.start_x+i >= out_image.image.size()) break;
      for (int j = std::floor(dy*i-step_size/2.); j <= std::floor(dy*i+step_size/2.); j++)
      {
        if (wind_after.start_y+j < 0) continue;
        if (wind_after.start_y+j >= out_image.image.at(i).size()) break;
        image_memory.at(wind_after.start_x+i).at(wind_after.start_y+j).insert(sel_index);
        color_before = out_image.image.at(wind_after.start_x+i).at(wind_after.start_y+j);
        color_after = wind_memory.at( *image_memory.at(wind_after.start_x+i).at(wind_after.start_y+j).rbegin() ).rgb;
        check = 0;
        for (int k = 0; k < 3; k++)
        {
          if (color_before.at(k) != color_after.at(k))
          {
            check = 1;
            break;
          }
        }
        if (check)
        {
          out_image.image.at(wind_after.start_x+i).at(wind_after.start_y+j) = color_after;
          for (int k = 0; k < 3; k++)
          {
            diff -= (long)(color_before.at(k) - ref_image.image.at(wind_after.start_x+i).at(wind_after.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind_after.start_x+i).at(wind_after.start_y+j).at(k));
            diff += (long)(color_after.at(k) - ref_image.image.at(wind_after.start_x+i).at(wind_after.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind_after.start_x+i).at(wind_after.start_y+j).at(k));
          }
        }
      }
    }
  }
  else
  {
    for (int i = 0; norm*i < wind_after.power ; i++)
    {
      if (wind_after.start_x-i < 0) break;
      for (int j = std::floor(-dy*i-step_size/2.); j <= std::floor(-dy*i+step_size/2.); j++)
      {
        if (wind_after.start_y+j < 0) continue;
        if (wind_after.start_y+j >= out_image.image.at(i).size()) break;
        image_memory.at(wind_after.start_x-i).at(wind_after.start_y+j).insert(sel_index);
        color_before = out_image.image.at(wind_after.start_x-i).at(wind_after.start_y+j);
        color_after = wind_memory.at( *image_memory.at(wind_after.start_x-i).at(wind_after.start_y+j).rbegin() ).rgb;
        check = 0;
        for (int k = 0; k < 3; k++)
        {
          if (color_before.at(k) != color_after.at(k))
          {
            check = 1;
            break;
          }
        }
        if (check)
        {
          out_image.image.at(wind_after.start_x-i).at(wind_after.start_y+j) = color_after;
          for (int k = 0; k < 3; k++)
          {
            diff -= (long)(color_before.at(k) - ref_image.image.at(wind_after.start_x-i).at(wind_after.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind_after.start_x-i).at(wind_after.start_y+j).at(k));
            diff += (long)(color_after.at(k) - ref_image.image.at(wind_after.start_x-i).at(wind_after.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind_after.start_x-i).at(wind_after.start_y+j).at(k));
          }
        }
      }
    }
  }
  sel_seed++;
}
void blow_wind(s_bmp &out_image, s_bmp &ref_image, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, long &diff)
{
  double norm, dy, step_size;
  s_wind wind;
  VC color_before, color_after;

  diff = 0;
  wind = get_wind(out_image.info.biWidth, out_image.info.biHeight, color_set);
  wind_memory.push_back(wind);
  dy = std::tan(wind.direc);
  step_size = std::min(std::abs(1./std::cos(wind.direc)),wind.power);
  norm = 1 + dy*dy;
  norm = std::sqrt(norm);
  if (wind.direc > -PI/2 || wind.direc <= PI/2)
  {
    for (int i = 0; norm*i < wind.power ; i++)
    {
      if (wind.start_x+i >= out_image.image.size()) break;
      for (int j = std::floor(dy*i-step_size/2.); j <= std::floor(dy*i+step_size/2.); j++)
      {
        if (wind.start_y+j < 0) continue;
        if (wind.start_y+j >= out_image.image.at(i).size()) break;
        image_memory.at(wind.start_x+i).at(wind.start_y+j).insert(wind_memory.size()-1);
        color_before = out_image.image.at(wind.start_x+i).at(wind.start_y+j);
        color_after = wind.rgb;
        out_image.image.at(wind.start_x+i).at(wind.start_y+j) = color_after;
        for (int k = 0; k < 3; k++)
        {
          diff -= (long)(color_before.at(k) - ref_image.image.at(wind.start_x+i).at(wind.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind.start_x+i).at(wind.start_y+j).at(k));
          diff += (long)(color_after.at(k) - ref_image.image.at(wind.start_x+i).at(wind.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind.start_x+i).at(wind.start_y+j).at(k));
        }
      }
    }
  }
  else
  {
    for (int i = 0; norm*i < wind.power ; i++)
    {
      if (wind.start_x-i < 0) break;
      for (int j = std::floor(-dy*i-step_size/2.); j <= std::floor(-dy*i+step_size/2.); j++)
      {
        if (wind.start_y+j < 0) continue;
        if (wind.start_y+j >= out_image.image.at(i).size()) break;
        image_memory.at(wind.start_x-i).at(wind.start_y+j).insert(wind_memory.size()-1);
        color_before = out_image.image.at(wind.start_x-i).at(wind.start_y+j);
        color_after = wind.rgb;
        out_image.image.at(wind.start_x-i).at(wind.start_y+j) = color_after;
        for (int k = 0; k < 3; k++)
        {
          diff -= (long)(color_before.at(k) - ref_image.image.at(wind.start_x-i).at(wind.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind.start_x-i).at(wind.start_y+j).at(k));
          diff += (long)(color_after.at(k) - ref_image.image.at(wind.start_x-i).at(wind.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind.start_x-i).at(wind.start_y+j).at(k));
        }
      }
    }
  }
}
