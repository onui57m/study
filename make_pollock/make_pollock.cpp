/**
 *
 * making pollocizied image (fractal)
 * It is assumed that ref_image is a 24-bit bitmap, and width and height are both 2^n.
 *
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-12-28T22:08:39+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-12-31T02:36:57+09:00
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
typedef std::vector<int> VI;
typedef std::vector<VI> VVI;
typedef std::vector<VVI> VVVI;
typedef std::vector<std::vector<std::set<int> > > VVSI;

struct s_wind
{
  int start_x;
  int start_y;
  double direc;
  double power;
  VC rgb;
};
struct s_frac
{
  double frac;
  double R;
  double var;
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
void initialize(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, double &score);
void draw_wind(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, s_wind wind, double &diff, int sel_index);
void erase_wind(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, s_wind wind, double &diff, int sel_index);
void k_means_pp(s_bmp &ref_image, int k_cluster, VVC &color_set);
void hill_climb(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, double &score, int calc_time);
void calc_fractality(int box_width, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, int x, int y, VC &color_before, VC &color_after, double &diff);
void linear_regression(s_frac &fractality, VI &box_count);

int main(int argc, char *argv[])
{
  s_bmp out_image, ref_image;
  std::vector<s_wind> wind_memory;
  int k_cluster, calc_time;
  s_frac fractality;
  double ref_fractality;
  double score;
  VVC color_set;
  VVVI each_count;
  VI box_count;
  VVSI image_memory;

  k_cluster = 8; /* the number of colors to use for drawing picture */
  ref_fractality = 1.9; /* a reference value to which a fractality is about to be converged */
  calc_time = 1; /* time for calculation [min] */
  if (argc > 1)
    k_cluster = std::atoi(argv[1]);
  if (argc > 2)
    ref_fractality = std::atof(argv[2]);
  if (argc > 3)
    calc_time = std::atoi(argv[3]);
  std::cout << "k_cluster = " << k_cluster << " ref_fractality = " << ref_fractality << ", calc_time = " << calc_time << " min\n";
  read_bmp(out_image, ref_image, image_memory);
  k_means_pp(ref_image, k_cluster, color_set);
  initialize(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, color_set, score);
  hill_climb(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, color_set, score, calc_time);
  write_bmp(out_image);
}

s_wind get_wind(int width, int height, VVC &color_set)
{
  static int gen_seed = 3141592;
  s_wind wind;
  std::mt19937 mt(gen_seed);
  std::uniform_real_distribution<double> dist_dir(-PI, PI);
  std::uniform_real_distribution<double> dist_pow(width/32., width/2.);
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
  unsigned char dummy;
  VC base_color(3,255);
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

  VVC one_line(ref_image.info.biWidth,base_color);
  std::vector<std::set<int> > blank_memory(ref_image.info.biWidth);
  for (int i = 0; i < ref_image.info.biHeight; i++)
  {
    ref_image.image.push_back(one_line);
    out_image.image.push_back(one_line);
    image_memory.push_back(blank_memory);
    for (int j = 0; j < ref_image.info.biWidth; j++)
    {
      for (int k = 0; k < 3; k++)
        fbmp.read((char *) &ref_image.image.at(i).at(j).at(k), sizeof(unsigned char));
    }
    if (ref_image.info.biWidth % 4)
    {
      for (int j = 0; j < 4 - out_image.info.biWidth*3 % 4; j++)
        fbmp.read((char *) &dummy, sizeof(unsigned char));
    }
  }
  fbmp.close();
  out_image.file = ref_image.file;
  out_image.info = ref_image.info;
}
void write_bmp(s_bmp &out_image)
{
  VC base_color(3,255);
  int count;
  unsigned char dummy;
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
  count = 0;
  for (int i = 0; i < out_image.info.biHeight; i++)
  {
    for (int j = 0; j < out_image.info.biWidth; j++)
    {
      for (int k = 0; k < 3; k++)
        fbmp.write((char *) &out_image.image.at(i).at(j).at(k), sizeof(unsigned char));
      if (out_image.image.at(i).at(j) == base_color)
        count++;
    }
    if (out_image.info.biWidth % 4)
    {
      for (int j = 0; j < 4 - out_image.info.biWidth*3 % 4; j++)
      {
        fbmp.write((char *) &dummy, sizeof(unsigned char));
      }
    }
  }
  fbmp.close();
  std::cout << "count = " << out_image.info.biWidth*out_image.info.biHeight-count << "\n";
}
void initialize(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, double &score)
{
  int wind_num, box_width, box_num;
  double diff;
  s_wind wind;

  box_width = out_image.info.biWidth;
  box_num = 1;
  while (box_width)
  {
    VVI one_set(box_num,VI(box_num,0));
    each_count.push_back(one_set);
    box_count.push_back(0);
    box_width /= 2;
    box_num *= 2;
  }
  wind_num = 100;
  score = 0;
  fractality.frac = 0;
  fractality.R = 1;
  fractality.var = 1;
  for (int i = 0; i < wind_num; i++)
  {
    wind = get_wind(out_image.info.biWidth, out_image.info.biHeight, color_set);
    diff = 0;
    draw_wind(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, wind, diff, i);
    score += diff;
  }
}
void draw_wind(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, s_wind wind, double &diff, int sel_index)
{
  int sign;
  double norm, dy, step_size;
  VC color_before, color_after, base_color(3,255);
  bool check;

  if (wind_memory.size() == sel_index)
    wind_memory.push_back(wind);
  else
    wind_memory.at(sel_index) = wind;
  dy = std::tan(wind.direc);
  step_size = std::min(std::abs(1./std::cos(wind.direc)),wind.power);
  norm = 1 + dy*dy;
  norm = std::sqrt(norm);
  if (wind.direc > -PI/2 && wind.direc <= PI/2)
    sign = 1;
  else
    sign = -1;
  for (int i = 0; norm*i < wind.power ; i++)
  {
    if (wind.start_x+sign*i >= out_image.image.size() || wind.start_x+sign*i < 0) break;
    for (int j = std::floor(dy*sign*i-step_size/2.); j <= std::floor(dy*sign*i+step_size/2.); j++)
    {
      if (wind.start_y+j < 0) continue;
      if (wind.start_y+j >= out_image.image.at(wind.start_x+sign*i).size()) break;
      image_memory.at(wind.start_x+sign*i).at(wind.start_y+j).insert(sel_index);
      color_before = out_image.image.at(wind.start_x+sign*i).at(wind.start_y+j);
      color_after = wind_memory.at( *image_memory.at(wind.start_x+sign*i).at(wind.start_y+j).rbegin() ).rgb;
      check = false;
      for (int k = 0; k < 3; k++)
      {
        if (color_before.at(k) != color_after.at(k))
        {
          check = true;
          break;
        }
      }
      if (check)
      {
        out_image.image.at(wind.start_x+sign*i).at(wind.start_y+j) = color_after;
        if (color_before == base_color)
          calc_fractality(out_image.info.biWidth, each_count, box_count, fractality, ref_fractality, wind.start_x+sign*i, wind.start_y+j, color_before, color_after, diff);
        for (int k = 0; k < 3; k++)
        {
          diff -= (long)(color_before.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k));
          diff += (long)(color_after.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k));
        }
      }
    }
  }
}
void erase_wind(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, s_wind wind, double &diff, int sel_index)
{
  int sign;
  double norm, dy, step_size;
  VC color_before, color_after, base_color(3,255);
  bool check;

  dy = std::tan(wind.direc);
  step_size = std::min(std::abs(1./std::cos(wind.direc)),wind.power);
  norm = 1 + dy*dy;
  norm = std::sqrt(norm);
  if (wind.direc > -PI/2 && wind.direc <= PI/2)
    sign = 1;
  else
    sign = -1;
  for (int i = 0; norm*i < wind.power ; i++)
  {
    if (wind.start_x+sign*i >= out_image.image.size() || wind.start_x+sign*i < 0) break;
    for (int j = std::floor(dy*sign*i-step_size/2.); j <= std::floor(dy*sign*i+step_size/2.); j++)
    {
      if (wind.start_y+j < 0) continue;
      if (wind.start_y+j >= out_image.image.at(wind.start_x+sign*i).size()) break;
      image_memory.at(wind.start_x+sign*i).at(wind.start_y+j).erase(sel_index);
      color_before = out_image.image.at(wind.start_x+sign*i).at(wind.start_y+j);
      if (image_memory.at(wind.start_x+sign*i).at(wind.start_y+j).size())
        color_after = wind_memory.at( *image_memory.at(wind.start_x+sign*i).at(wind.start_y+j).rbegin() ).rgb;
      else
        color_after = base_color;
      check = false;
      for (int k = 0; k < 3; k++)
      {
        if (color_before.at(k) != color_after.at(k))
        {
          check = true;
          break;
        }
      }
      if (check)
      {
        out_image.image.at(wind.start_x+sign*i).at(wind.start_y+j) = color_after;
        if (color_after == base_color)
          calc_fractality(out_image.info.biWidth, each_count, box_count, fractality, ref_fractality, wind.start_x+sign*i, wind.start_y+j, color_before, color_after, diff);
        for (int k = 0; k < 3; k++)
        {
          diff -= (long)(color_before.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k))*(long)(color_before.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k));
          diff += (long)(color_after.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k))*(long)(color_after.at(k) - ref_image.image.at(wind.start_x+sign*i).at(wind.start_y+j).at(k));
        }
      }
    }
  }
  wind_memory.at(sel_index).rgb = base_color;
}
void k_means_pp(s_bmp &ref_image, int k_cluster, VVC &color_set)
{
  std::mt19937 mt(653589);
  std::uniform_int_distribution<int> dist_center(0, ref_image.info.biWidth*ref_image.info.biHeight-1);
  std::vector<long> prob(ref_image.info.biWidth*ref_image.info.biHeight);
  VI blank, label(ref_image.info.biWidth*ref_image.info.biHeight,0), pre_label(ref_image.info.biWidth*ref_image.info.biHeight,0);
  VVI clusters(k_cluster), sum_color(k_cluster, VI(3,0));
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
void hill_climb(s_bmp &out_image, s_bmp &ref_image, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, VVSI &image_memory, std::vector<s_wind> &wind_memory, VVC &color_set, double &score, int calc_time)
{
  std::mt19937 mt(793238);
  std::uniform_int_distribution<int> dist_que(0, 2);
  int que, sel_index;
  struct timeval tmp_time;
  long long init_time, fin_time, current_time;
  double diff;
  s_bmp old_image;
  s_wind wind1, wind2;
  std::vector<s_wind> old_wind_memory;
  VVSI old_image_memory;
  VVVI old_each_count;
  VI old_box_count;
  s_frac old_frac;
  VC base_color(3,255);
  std::ofstream fscore;

  gettimeofday(&tmp_time, NULL);
  init_time = tmp_time.tv_sec*1000000 + tmp_time.tv_usec;
  current_time = init_time;
  fin_time = init_time + 60000000 * calc_time; /* calc_time [min] */
  fscore.open("score.dat");

  while (current_time < fin_time)
  {
    gettimeofday(&tmp_time, NULL);
    current_time = tmp_time.tv_sec*1000000 + tmp_time.tv_usec;
    fscore << current_time << " " << score << " " << fractality.frac << " " << fractality.R << " " << fractality.var << "\n";
    for (int i = 0; i < 1; i++)
    {
      old_image = out_image;
      old_wind_memory = wind_memory;
      old_image_memory = image_memory;
      old_each_count = each_count;
      old_box_count = box_count;
      old_frac = fractality;
      diff = 0;
      que = dist_que(mt);
      if (que == 0)
      {
        std::uniform_int_distribution<int> dist_memory(0, wind_memory.size()-1);
        sel_index = dist_memory(mt);
        while (wind_memory.at(sel_index).rgb == base_color)
          sel_index = dist_memory(mt);
        wind1 = wind_memory.at(sel_index);
        wind2 = get_wind(out_image.info.biWidth, out_image.info.biHeight, color_set);
        erase_wind(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, wind1, diff, sel_index);
        draw_wind(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, wind2, diff, sel_index);
      }
      else if (que == 1)
      {
        wind1 = get_wind(out_image.info.biWidth, out_image.info.biHeight, color_set);
        draw_wind(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, wind1, diff, wind_memory.size());
      }
      else
      {
        std::uniform_int_distribution<int> dist_memory(0, wind_memory.size()-1);
        sel_index = dist_memory(mt);
        while (wind_memory.at(sel_index).rgb == base_color)
          sel_index = dist_memory(mt);
        wind1 = wind_memory.at(sel_index);
        erase_wind(out_image, ref_image, each_count, box_count, fractality, ref_fractality, image_memory, wind_memory, wind1, diff, sel_index);
      }
      if (diff >= 0)
      {
        out_image = old_image;
        wind_memory = old_wind_memory;
        image_memory = old_image_memory;
        each_count = old_each_count;
        box_count = old_box_count;
        fractality = old_frac;
      }
      else
        score += diff;
    }
  }
  gettimeofday(&tmp_time, NULL);
  current_time = tmp_time.tv_sec*1000000 + tmp_time.tv_usec;
  fscore << current_time << " " << score << " " << fractality.frac << " " << fractality.R << " " << fractality.var << "\n";
  fscore.close();
}
void calc_fractality(int box_width, VVVI &each_count, VI &box_count, s_frac &fractality, double ref_fractality, int x, int y, VC &color_before, VC &color_after, double &diff)
{
  int box_index, box_x, box_y;
  long frac_score, R_score, var_score;
  double variance_box, mean_box;
  VC base_color(3,255);
  s_frac pre_fractality;

  box_index = 0;
  pre_fractality = fractality;
  while (box_width)
  {
    box_x = x/box_width;
    box_y = y/box_width;
    if (color_before == base_color)
    {
      each_count.at(box_index).at(box_x).at(box_y)++;
      if (each_count.at(box_index).at(box_x).at(box_y) == 1)
        box_count.at(box_index)++;
    }
    else
    {
      each_count.at(box_index).at(box_x).at(box_y)--;
      if (each_count.at(box_index).at(box_x).at(box_y) == 0)
        box_count.at(box_index)--;
    }
    box_width /= 2;
    box_index++;
  }
  mean_box = 0;
  variance_box = 0;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      mean_box += each_count.at(3).at(i).at(j);
      variance_box += each_count.at(3).at(i).at(j)*each_count.at(3).at(i).at(j);
    }
  }
  mean_box /= 64.;
  variance_box /= 64.;
  variance_box -= mean_box*mean_box;
  fractality.var = variance_box;

  frac_score = 10000000000;
  var_score = 100000;
  // R_score = 10*frac_score;
  linear_regression(fractality, box_count);
  diff -= std::abs(pre_fractality.frac - ref_fractality)*frac_score + pre_fractality.var*var_score;
  diff += std::abs(fractality.frac - ref_fractality)*frac_score + fractality.var*var_score;
  // diff += (pre_fractality.R - fractality.R)*R_score;
}
void linear_regression(s_frac &fractality, VI &box_count)
{
  double variance_x, variance_y, covariance, slope, segment, mean_x, mean_y, square_diff, R;
  std::vector<double> y;

  mean_x = 0;
  mean_y = 0;
  variance_x = 0;
  variance_y = 0;
  covariance = 0;
  for (int i = 0; i < box_count.size(); i++)
  {
    y.push_back(std::log2(box_count.at(i)));
    mean_x += box_count.size()-1-i;
    mean_y += y.at(i);
    variance_x += (box_count.size()-1-i)*(box_count.size()-1-i);
    variance_y += y.at(i)*y.at(i);
    covariance += (box_count.size()-1-i)*y.at(i);
  }
  mean_x /= box_count.size();
  mean_y /= box_count.size();
  variance_x /= box_count.size();
  variance_y /= box_count.size();
  covariance /= box_count.size();
  variance_x -= mean_x*mean_x;
  variance_y -= mean_y*mean_y;
  covariance -= mean_x*mean_y;
  if (variance_x != 0)
    slope = covariance/variance_x;
  else
    slope = INFL;
  segment = mean_y - slope*mean_x;
  square_diff = 0;
  for (int i = 0; i < box_count.size(); i++)
    square_diff += (y.at(i) - slope*i - segment)*(y.at(i) - slope*i - segment);
  square_diff /= box_count.size();
  R = 1-square_diff/variance_y;

  fractality.frac = -slope;
  fractality.R = R;
}
