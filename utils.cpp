#include"utils.h"
#include<math.h>
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

int LoadSurfBin(const char *filename, int *fealen,float **features)
{
	FILE *fid = fopen(filename,"rb");
	if(fid== NULL)
	{
		printf("I/O error: Unable to open the file %s\n",filename);
		return -1;
	}
	float version; 
	int featype,dim,len;
	fread(&version,sizeof(float),1,fid);
	fread(&featype,sizeof(int),1,fid);
	if(featype != 1)
	{
		printf("Error:the feature type isn't surf,featype = %d\n",featype);
		return -1;
	}
	fread(&dim,sizeof(int),1,fid);
	fread(&len,sizeof(int),1,fid);
	float *features_in = (float *)calloc(len * dim, sizeof(float));
	if(features_in == NULL)
	{
		printf("Can't apply enough memory!\n");
		return -1;
	}
	float x,y,scale,orientation;
	int laplacian;
	float *temp;
	for(int i = 0;i != len;++i)
	{
		temp = features_in + i * dim;
		fread(&x,sizeof(float),1,fid);
		fread(&y,sizeof(float),1,fid);
		fread(&scale,sizeof(float),1,fid);
		fread(&orientation,sizeof(float),1,fid);
		fread(&laplacian,sizeof(int),1,fid);
		fread(temp,sizeof(float),dim,fid);
	}
	*fealen = len;
	*features = features_in;
	fclose(fid);
	return 0;
}

// param
// return: 0, 正确返回
//		  -1, 此文件的错误
int LoadVlad(const char *filename,int &kfNum,float *&features,int *&frameid_out)
{
	FILE* featureFile = fopen(filename,"rb");
	if(featureFile == NULL)
	{
		printf("Can't Open file %s\n",filename);
		return -1;
	}
	float version = 0.0;
	int featype = 0, dim = 0;
	fread(&version,sizeof(float),1,featureFile);
	fread(&featype,sizeof(int),1,featureFile);
	fread(&dim,sizeof(int),1,featureFile);
	if(featype != 10 || dim != kSurfDescriptorDim * kCentroidsNum)
	{
		printf("This File is't vlad features file or the dim is't right!\n");
		fclose(featureFile);
		return -1;
	}
	int count = fread(&kfNum,sizeof(int),1,featureFile);
	if (count!=1 || kfNum==0)
	{
		printf("Read smpfile %s error:count = %d, kfNum = %d\n",filename,count,kfNum);
		fclose(featureFile);
		return -1;
	}
	
	float *pCoarseFeature = (float*)calloc(kfNum * dim,sizeof(float));
	if (pCoarseFeature==NULL)
	{
		printf("failed to allocate memory for feature\n");
		fclose(featureFile);
		return -1;
	}
	int *frameid = (int *)calloc(kfNum,sizeof(int));
	if(frameid == NULL)
	{
		printf("failed to allocate memory for feature\n");
		fclose(featureFile);
		free(pCoarseFeature);
		return -1;
	}
	for(int i = 0;i != kfNum;++i)
	{
		fread(frameid + i,sizeof(int),1,featureFile);
		count = fread(pCoarseFeature + i * dim,sizeof(float),dim,featureFile);
		if (count != dim)
		{
			printf("Read smpfile %s length error:count = %d, kfNum = %d\n",filename,count,kfNum);
			free(pCoarseFeature);
			free(frameid);
			fclose(featureFile);
			return -1;
		}
	}
	fclose(featureFile);
	
//	printf("before normalize, norm = %.2f\n",FvecNorm2(pCoarseFeature,kfNum * dim));
//	// power normalization
//	double alpha = 0.5;
//	FvecSignPow(pCoarseFeature,kfNum * dim,alpha);
//	// L2 normalization
//	FvecsNormalize2(pCoarseFeature,kfNum,dim);
//	printf("after normalize, norm = %.2f\n",FvecNorm2(pCoarseFeature,kfNum * dim));

	features = pCoarseFeature;
	if(frameid_out) frameid_out = frameid;
	else free(frameid);
	return 0;
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

// L1 Normalize(original features has unit L2 norm) + square root each element
// param:
//	n 	the number of features
//	d	the dimension of features
// remark:
// 	针对负值情况,参考http://blog.csdn.net/axman/article/details/8979508处理
//	"没有必要，只要注意总和变成正数就行了"
void RootOperate(float *features, int n, int d)
{
	int i,j;
	float eps = 1E-12;
	for(i = 0;i != n;++i)
	{
		//fvec_normalize(features + i * d,d,1);	// L1 normalize
		float sum = 0.0;
		for(j = 0;j != d; ++j)
			sum += fabs(features[i*d+j]);
		for(j = 0; j != d;++j)
		{
			if(features[i*d+j] < 0) features[i * d + j] = (float) -sqrt(-features[i * d + j] / (sum + eps)); // 防止sum为0
			else features[i * d + j] = (float) sqrt(features[i * d + j] / (sum + eps));
		}
	}
}

void Normalize2(float *v,int nv)
{
	int i = 0;
	float sum = 0.0;
	for(i = 0;i < nv;++i)
	{
		sum += v[i] * v[i];
	}
	sum = sqrt(sum);
	if(sum == 0)
	{
		for(i = 0;i < nv;++i)
			v[i] = 1.0 / sqrt(nv);
	}
	else
	{
		for(i = 0;i < nv;++i)
			v[i] /= sum;
	}
}

void FvecsNormalize2(float *v,int n,int d)
{
	for(int i = 0;i < n;++i)
	{
		Normalize2(v + i * d,d);
	}
}

void FvecSignPow(float * v, long n, double scal)
{
  	long i;
  	for (i = 0 ; i < n ; i++)
    	v[i] = v[i]>0 ? pow(v[i],scal) : -pow(-v[i],scal);
}

float FvecNorm2(const float *v, int n)
{
	int i;
	float sum = 0.0;
	for(i = 0;i != n;++i)
		sum += v[i] * v[i];
	return sqrt(sum);
}

