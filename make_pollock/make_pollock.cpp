/**
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-12-28T22:08:39+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-12-29T01:52:57+09:00
 */

#include <iostream>
#include <fstream>
#include <random>
#include <vector>

#define PI 3.141592653589703238462

typedef std::vector<char> VC;
typedef std::vector<VC> VVC;

struct s_wind
{
  int start_x;
  int start_y;
  double direc;
  double power;
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
  VVC image;
};

s_wind get_wind(int &seed, int width, int height);
void read_bmp(s_bmp &out_image, s_bmp &ref_image);
void write_bmp(s_bmp &out_image, s_bmp &ref_image);

int main()
{
  int width, height, seed, wind_num;
  s_wind wind;
  s_bmp out_image, ref_image;

  seed = 0;
  wind_num = 100;
  read_bmp(out_image, ref_image);
  width = out_image.info.biWidth;
  height = out_image.info.biHeight;
  for (int i = 0; i < wind_num; i++)
  {
    wind = get_wind(seed, width, height);

  }
  write_bmp(out_image, ref_image);
}

s_wind get_wind(int &seed, int width, int height)
{
  std::mt19937 mt(seed);
  std::uniform_real_distribution<double> wind_dir(-PI, PI);
  std::uniform_real_distribution<double> wind_pow(1., 5.);
  std::uniform_int_distribution<int> wind_x(0, width-1);
  std::uniform_int_distribution<int> wind_y(0, height-1);

  s_wind wind;

  wind.direc = wind_dir(mt);
  wind.power = wind_pow(mt);
  wind.start_x = wind_x(mt);
  wind.start_y = wind_y(mt);
  seed++;
  return wind;
}
void read_bmp(s_bmp &out_image, s_bmp &ref_image)
{
  char dummy;
  std::ifstream fbmp;
  fbmp.open("ref.bmp", std::ios::binary);
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

  VC one_line(ref_image.info.biWidth*3);
  for (int i = 0; i < ref_image.info.biHeight; i++)
  {
    ref_image.image.push_back(one_line);
    for (int j = 0; j < ref_image.info.biWidth*3; j++)
    {
      fbmp.read((char *) &ref_image.image.at(i).at(j), sizeof(char));
    }
    while (ref_image.image.at(i).size() % 4 != 0)
    {
      fbmp.read((char *) &dummy, sizeof(char));
      ref_image.image.at(i).push_back(dummy);
    }
  }
  fbmp.close();
  out_image.file = ref_image.file;
  out_image.info = ref_image.info;
  out_image.image = ref_image.image;
  for (int i = 0; i < out_image.info.biHeight; i++)
  {
    for (int j = 0; j < out_image.image.at(i).size(); j++)
      out_image.image.at(i).at(j) = 0;
  }
}
void write_bmp(s_bmp &out_image, s_bmp &ref_image)
{
  char dummy;
  std::ofstream fbmp;
  fbmp.open("fractal.bmp", std::ios::binary);
  if (!fbmp)
  {
    std::cerr << "Cannot open a file.\n";
    exit(1);
  }
  fbmp.write((char *) &ref_image.file.bfType, sizeof(unsigned short));
  fbmp.write((char *) &ref_image.file.bfSize, sizeof(unsigned int));
  fbmp.write((char *) &ref_image.file.bfReserved1, sizeof(unsigned short));
  fbmp.write((char *) &ref_image.file.bfReserved2, sizeof(unsigned short));
  fbmp.write((char *) &ref_image.file.bfOffBits, sizeof(unsigned int));
  fbmp.write((char *) &ref_image.info.biSize, sizeof(unsigned int));
  fbmp.write((char *) &ref_image.info.biWidth, sizeof(int));
  fbmp.write((char *) &ref_image.info.biHeight, sizeof(int));
  fbmp.write((char *) &ref_image.info.biPlanes, sizeof(unsigned short));
  fbmp.write((char *) &ref_image.info.biBitCount, sizeof(unsigned short));
  fbmp.write((char *) &ref_image.info.biCompression, sizeof(unsigned int));
  fbmp.write((char *) &ref_image.info.biSizeImage, sizeof(unsigned int));
  fbmp.write((char *) &ref_image.info.biXPixPerMeter, sizeof(int));
  fbmp.write((char *) &ref_image.info.biYPixPerMeter, sizeof(int));
  fbmp.write((char *) &ref_image.info.biClrUsed, sizeof(unsigned int));
  fbmp.write((char *) &ref_image.info.biCirImportant, sizeof(unsigned int));
  if (ref_image.info.biBitCount < 24)
  {
    fbmp.write((char *) &ref_image.rgb.rgbBlue, sizeof(unsigned char));
    fbmp.write((char *) &ref_image.rgb.rgbGreen, sizeof(unsigned char));
    fbmp.write((char *) &ref_image.rgb.rgbRed, sizeof(unsigned char));
    fbmp.write((char *) &ref_image.rgb.rgbReserved, sizeof(unsigned char));
  }

  dummy = 0;
  for (int i = 0; i < out_image.info.biHeight; i++)
  {
    for (int j = 0; j < out_image.info.biWidth*3; j++)
    {
      fbmp.write((char *) &out_image.image.at(i).at(j), sizeof(char));
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
