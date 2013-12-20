#include"utils.h"
#include<stdlib.h>
#include<string.h>
#include<fstream>
#include<vector>
extern "C" { 
#include <yael/vector.h>
}
#include<global.h>

bool LoadSurfDescriptor(const char *filename,float *&descriptor,int &descriptor_num)
{
	descriptor = new float[kSurfDescriptorDim * kMaxSurfDescriptorNUM]();
	if(descriptor == NULL)
	{
		printf("Can't apply memeory\n");
		return false;
	}
  	std::ifstream infile(filename);
	float x,y,scale,orientation;
	int laplacian;
	int dim,count;
	// read descriptor length/number of ipoints
	infile >> dim;
	infile >> count;
	if(count > kMaxSurfDescriptorNUM) count = kMaxSurfDescriptorNUM;
	descriptor_num = count;
	for(int i = 0;i != count;++i)
	{
		// read vals
    	infile >> scale; 
    	infile >> x;
    	infile >> y;
    	infile >> orientation;
    	infile >> laplacian;
    	infile >> scale;
		for(int j = 0;j != kSurfDescriptorDim;++j)
			infile >> descriptor[i * kSurfDescriptorDim + j];
	}
	return true;
}

bool LoadSiftgeo(const char *filename,float *&descriptor,int &descriptor_num)
{
	unsigned char *siftgeo;
	int dim;
	if((descriptor_num = bvecs_new_from_siftgeo(filename,&dim,&siftgeo,NULL,NULL)) < 0)
	{
		free(siftgeo);
		return false;
	}
	descriptor = bvec2fvec(siftgeo,descriptor_num * dim);
	free(siftgeo);
	return true;
}
