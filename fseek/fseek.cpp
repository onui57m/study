/**
 * @Author: Mizuki Onui <onui_m>
 * @Date:   2020-10-07T03:27:20+09:00
 * @Last modified by:   onui_m
 * @Last modified time: 2020-10-07T04:22:00+09:00
 */

#include <iostream>
#include <fstream>
#include <cstdio>

int main()
{
  int tmp;
  FILE *fout, *fin;
  std::ofstream ofs;
  std::ifstream ifs;

  /* ++++++++++ test1 ++++++++++ */
    fout=fopen("test1.in","wb");
    for (int i = 0; i < 10; i++)
      fwrite(&i, sizeof(int), 1, fout);
    fclose(fout);

    fin=fopen("test1.in","rb");
    fseek(fin,sizeof(int)*2,SEEK_SET);
    fread(&tmp,sizeof(int),1,fin);
    fout=fopen("test1.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);

    ofs.open("test1p.in", std::ios_base::binary);
    for (int i = 0; i < 10; i++)
      ofs.write((char *)&i, sizeof(int));
    ofs.close();
    ifs.open("test1p.in", std::ios_base::binary);
    ifs.seekg(sizeof(int)*2,std::ios::beg);
    ifs.read((char *)&tmp, sizeof(int));
    ofs.open("test1p.out");
    ofs << tmp << "\n";
    ifs.close();
    ofs.close();
  /* ++++++++++ test1 ++++++++++ */
  /* ++++++++++ test2 ++++++++++ */
    fout=fopen("test2.in","w");
    for (int i = 0; i < 10; i++)
      fprintf(fout,"%d",i);
    fclose(fout);

    fin=fopen("test2.in","r");
    fscanf(fin,"%d",&tmp);
    fout=fopen("test2.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);

    ofs.open("test2p.in");
    for (int i = 0; i < 10; i++)
      ofs << i;
    ofs.close();
    ifs.open("test2p.in");
    ifs >> tmp;
    ofs.open("test2p.out");
    ofs << tmp << "\n";
    ifs.close();
    ofs.close();
  /* ++++++++++ test2 ++++++++++ */
  /* ++++++++++ test3 ++++++++++ */
    fout=fopen("test3.in","w");
    for (int i = 0; i < 10; i++)
      fprintf(fout,"%d",i);
    fclose(fout);

    fin=fopen("test3.in","r");
    fseek(fin,sizeof(int)*2,SEEK_SET);
    fscanf(fin,"%d",&tmp);
    fout=fopen("test3.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);

    ofs.open("test3p.in");
    for (int i = 0; i < 10; i++)
      ofs << i;
    ofs.close();
    ifs.open("test3p.in");
    ifs.seekg(sizeof(int)*2,std::ios::beg);
    ifs >> tmp;
    ofs.open("test3p.out");
    ofs << tmp << "\n";
    ifs.close();
    ofs.close();
  /* ++++++++++ test3 ++++++++++ */
  /* ++++++++++ test4 ++++++++++ */
    fout=fopen("test4.in","w");
    for (int i = 0; i < 10; i++)
      fprintf(fout,"%d ",i);
    fclose(fout);

    fin=fopen("test4.in","r");
    fscanf(fin,"%d",&tmp);
    fout=fopen("test4.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);
  /* ++++++++++ test4 ++++++++++ */
  /* ++++++++++ test5 ++++++++++ */
    fout=fopen("test5.in","w");
    for (int i = 0; i < 10; i++)
      fprintf(fout,"%d ",i);
    fclose(fout);

    fin=fopen("test5.in","r");
    fseek(fin,sizeof(int),SEEK_SET);
    fscanf(fin,"%d",&tmp);
    fout=fopen("test5.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);
  /* ++++++++++ test5 ++++++++++ */
  /* ++++++++++ test6 ++++++++++ */
    fout=fopen("test6.in","w");
    for (int i = 10; i < 20; i++)
      fprintf(fout,"%d ",i);
    fclose(fout);

    fin=fopen("test6.in","r");
    fseek(fin,sizeof(int),SEEK_SET);
    fscanf(fin,"%d",&tmp);
    fout=fopen("test6.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);
  /* ++++++++++ test6 ++++++++++ */
  /* ++++++++++ test7 ++++++++++ */
    fout=fopen("test7.in","w");
    for (int i = 0; i < 10; i++)
      fprintf(fout,"%d.0 ",i);
    fclose(fout);

    fin=fopen("test7.in","r");
    fseek(fin,sizeof(int)*2,SEEK_SET);
    fscanf(fin,"%d",&tmp);
    fout=fopen("test7.out","w");
    fprintf(fout,"%d\n",tmp);
    fclose(fin);
    fclose(fout);
  /* ++++++++++ test7 ++++++++++ */


}
