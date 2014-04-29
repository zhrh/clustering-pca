#ifndef UTILS_H
#define UTILS_H

extern int LoadSurfBin(const char *filename, int *fealen,float **features);
extern bool LoadSurfDescriptor(const char *filename,float *&descriptor,int &descriptor_num);
extern bool LoadSiftgeo(const char *filename,float *&descriptor,int &descriptor_num);

extern int LoadVlad(const char *filename,int &kfNum,float *&features,int *&frameid_out);

extern void RootOperate(float *features, int n, int d);

extern void Normalize2(float *v,int nv);
extern void FvecsNormalize2(float *v,int n,int d);
extern void FvecSignPow(float * v, long n, double scal);
extern float FvecNorm2(const float *v, int n);
#endif
