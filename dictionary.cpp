#include"dictionary.h"
#include<dirent.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<vector>
#include<algorithm>
extern "C" { 
#include<yael/kmeans.h>
#include <yael/vector.h>
}
#include"global.h"
#include"utils.h"

bool Dictionary::Create(const char *dir,int clusters,int dim,int points,float *&centroids)
{
	if(dir == NULL || clusters <= 0 || dim <= 0|| points <= 0 || centroids == NULL)
	{
		printf("invalid parameters\n");
		return false;
	}
	struct timeval start,end;
	int time_used;
	printf("analyzing files from the image directory...\n");
	gettimeofday(&start,NULL);
	std::vector<std::string> filepath;
	filepath.clear();
	ReadSurfPath(dir,filepath);
	if(filepath.empty())
	{
		printf("Don't find any features file!\n");
		return false;
	}
	printf("%d featurs file find\n",filepath.size());
	// we need the images to be representative for the entire set. images that are located in the same directory and
	// thus placed after each other often share similarities (e.g when the images were downloaded from the web they 
	// might have been found on the same website, and thus they can be closely related, or when downloading from flickr they
	// might be from the same user that took photos from the same scene).
	printf("randomizing image list...\n");
	random_shuffle(filepath.begin(),filepath.end());
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	printf("creating subset of points...\n");
	gettimeofday(&start,NULL);
	float *subset_desc;
	int subset_num;
	if(!DetermineSubset(filepath,dim,points,subset_desc,subset_num))
	{
		printf("Determine subset descriptors failed!\n");
		delete []subset_desc;
		return false;
	}
	printf("%d's points random choosed\n",subset_num);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);
	if(subset_num < clusters)
	{
		printf("number of points extract is smaller than the number of clusters\n");
		delete []subset_desc;
		return false;
	}

	printf("creating centroids...\n");
	gettimeofday(&start,NULL);
	int niter = 100;
	int redo = 3;
	centroids = new float[clusters * dim]();
	if(!PerformClustering(dim,clusters,niter,redo,subset_desc,subset_num,centroids))
	{
		printf("perform clustering failed\n");
		delete []centroids;
		delete []subset_desc;
		return false;
	}
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);
	delete []subset_desc;
	return true;
}
// determine subset of interest points
// param:
//	points, the number of points of each file saved
bool Dictionary::DetermineSubset(const std::vector<std::string> &filepath,int dim,int points,float *&subset,int &subset_num)
{
	float *subf= new float[filepath.size() * points * dim](); 	
	//float *subf= new float[filepath.size() * dim]();	// just for image vlad, because vlad file have only one features, zrh0222
	if(subf == NULL)
	{
		fprintf(stderr, "Apply memory failed!\n");
		return false;
	}
	//float *subf= new float[filepath.size() * kMaxSurfDescriptorNUM * dim](); 	
	unsigned int subp = 0;
	float *desc;
	int desc_num = 0;
	unsigned int *frameid = NULL;
	for(std::vector<std::string>::const_iterator iter = filepath.begin();iter != filepath.end();++iter)
	{
		//if(!LoadSurfDescriptor((*iter).c_str(),desc,desc_num))
		//if(!LoadSiftgeo((*iter).c_str(),desc,desc_num))
		//if(LoadSurfBin((*iter).c_str(),&desc_num,&desc) < 0)
		//if(LoadVlad((*iter).c_str(), desc_num, desc, frameid) < 0)
		//if(LoadBatchPcaVlad((*iter).c_str(), desc_num, desc, frameid) < 0)
		if(!LoadVideoPcaVlad((*iter).c_str(), desc_num, desc, frameid))
		{
			printf("Can't load surf descriptor from %s\n",(*iter).c_str());
			delete []subset;
			return false;
		}
		if(desc_num == 0)
			continue;
		//RootOperate(desc,desc_num,kSurfDescriptorDim);

		float *temp = subf + subp *dim;
		ExtractRandomPoints(dim,points,desc,desc_num,temp);
		//ExtractRandomPoints(dim,desc_num,desc,desc_num,temp);
		subp += desc_num;
		// delete []desc;
		free(desc);
		free(frameid); // for BatchPcavlad
	}
	subset = subf;
	subset_num = subp;
	return true;
}



void Dictionary::ExtractRandomPoints(int dim,int points,const float *src,int &desc_num,float *dst)
{
	if(desc_num > points)
	{
		// we don't use all descriptors as that will be too computationally intensive for the clustering algorithm.
		// thus we shuffle a list and randomly select the requested number of points 
		unsigned int *shuffle = new unsigned int[desc_num]();
		for(int i = 0;i < desc_num;++i)
			shuffle[i] = i;
		std::random_shuffle(shuffle, shuffle + desc_num);
		float *temp = dst;
		for(int i = 0;i != points;++i, temp += dim)
			memcpy(temp,src + shuffle[i] * dim,dim * sizeof(float));
		delete []shuffle;
		desc_num = points;
	}
	else
	{
		// if there are less points than we would like, which will occasionally happen,
		// use all points
		memcpy(dst,src,desc_num * dim * sizeof(float));
	}
}

// kmeans complexity: O(n*k*i) where n is the number of observations, k is the number of clusters and 
// i is the number of required iterations until convergence.
// param:
// 	clusters, the number of clusters
//	niter,	the number of iterations
bool Dictionary::PerformClustering(int dim,int clusters,int niter,int redo,const float *desc,int desc_num,float *centroids)
{
	long seed = 0;			// if seed !=0, the kmeans will acoording to the define seed to generator random numbers, for test
	//int flag = 0x60000;		// kmeans++ init + L2 normlization
	int flag = 0x20000;		// kmeans++ init
	float *dis = NULL;		// Don't need these return
	int *assign = NULL, *nassign = NULL;	// Don't need these return
	if(kmeans(dim,desc_num,clusters,niter,desc,flag,seed,redo,centroids,dis,assign,nassign) < 0.0)
		return false;
	return true;
}

bool Dictionary::ReadSurfPath(const char *surfdir,std::vector<std::string> &filepath)
{
	DIR *dir;
	struct dirent *file; 	//readdir函数的返回值就存放在这个结构体中
	struct stat st;
	static int filenum = 0;
	if(!(dir = opendir(surfdir)))
	{
		printf("error opendir %s!!!\n",surfdir);
		return false;
	}
	while((file = readdir(dir)) != NULL)
	{
		if((strcmp(file->d_name,".") == 0) || (strcmp(file->d_name,"..") == 0))
      		continue;

		char tmp_path[1024];
		memset(tmp_path,0,1024);
		sprintf(tmp_path,"%s/%s",surfdir,file->d_name);
		printf("%s\n",tmp_path);
		if(stat(tmp_path,&st) >= 0 && S_ISDIR(st.st_mode))
		{
			ReadSurfPath(tmp_path,filepath);	// 子目录继续递归
		}
		else
		{	++filenum;
			//if(filenum % 40 == 0)
			{
				char *sub = strrchr(tmp_path,'.');
				//char surfix[10];
				//memset(surfix,0,10);
				//strncpy(surfix,sub + 1, strlen(sub) - 1);
				if (!strcmp(sub + 1,"surf") || !strcmp(sub + 1,"siftgeo") || !strcmp(sub + 1,"vlad") ) // for test sift features
				{
					std::string path(tmp_path);
					filepath.push_back(path);
				}
			}
		}
	}
	closedir(dir);
	return true;
}

void Dictionary::Save(const char *fname,const int d,const int n,const float *centroids)
{
	fvecs_write(fname,d,n,centroids);
}

void Dictionary::CreateFromDic(const char *dic_fname, const char *cen_fname)
{
	float *v = (float *)malloc(sizeof(float) * kVisualWordsNum * kSurfDescriptorDim);
	ReadDic(dic_fname, kSurfDescriptorDim, kVisualWordsNum, v);
	float * centroids = fvec_new_0 (kCentroidsNum * kSurfDescriptorDim); 	/* output: centroids */
  	float * dis = fvec_new (kVisualWordsNum);           						/* point-to-cluster distance */
  	int * assign = ivec_new (kVisualWordsNum);          									/* quantization index of each point */
  	int * nassign = ivec_new (kCentroidsNum);
	kmeans(kSurfDescriptorDim,kVisualWordsNum,kCentroidsNum,0,v,0xC0000,1,1,centroids,dis,assign,nassign);
	fvecs_write(cen_fname,kSurfDescriptorDim,kCentroidsNum,centroids);

	free(v);
	free(centroids);
	free(dis);
	free(assign);
	free(nassign);
}

bool Dictionary::ReadDic(const char *fname, int d, int n, float *v)
{
	FILE *f_dic = fopen(fname,"rb");
	if(!f_dic)
	{
		fprintf (stderr, "ReadDic: could not open %s\n", fname);
    	perror ("");
    	return false;
	}
	if(fread(v, sizeof(float), d * n, f_dic) != d * n)
	{
		fprintf (stderr, "ReadDic error\n");
      	fclose(f_dic);
      	return false;	
	}
	fclose(f_dic);
	return true;
}
