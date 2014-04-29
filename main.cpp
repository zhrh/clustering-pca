#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dictionary.h"
#include "global.h"
#include "pca_transform.h"

const int kCentroidsNum = 64;
const int kSurfDescriptorDim = 64;
const int kMaxSurfDescriptorNUM = 5000;
const int kVisualWordsNum = 100000;
const int kSiftDescriptorDim = 128;
const int kMaxFileNum = 3000;
//const int kMaxSiftgeoNum = 10000000;

// /media/gxg_disk/zhourenhao/crawl_300k_surf/gpusurf_fea 
// ./result/clust_surf_k64_image.fvecs

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		printf("Para Error!\n");
        // 1:features path or dictionary path, 2:centroids path
		printf("Dictionary Use: 1.features/dictionary path 2.centroids path\n"); 
		printf("PCA Transform Use: 1.features dir 2. pca matrix path\n"); 
		return -1;
	}
	srand( (unsigned)time(NULL) );
	int points = 10000;
//	float *centroids;
//	if(Dictionary::Create(argv[1],kCentroidsNum,kSurfDescriptorDim,points,centroids))
//	{
//		Dictionary::Save(argv[2],kSurfDescriptorDim,kCentroidsNum,centroids);
//		delete []centroids;
//	}

	float *pca_mean,*pca_proj;
	//if(PCATransform::Create(argv[1],kSurfDescriptorDim * kCentroidsNum,points,pca_mean,pca_proj))
	if(PCATransform::CreateFromFvecs(argv[1],5 * kSiftDescriptorDim * kCentroidsNum, points, pca_mean, pca_proj))
	{
		PCATransform::Save(argv[2],5 * kSiftDescriptorDim * kCentroidsNum,pca_mean,pca_proj);
		delete []pca_mean;
		delete []pca_proj;
	}

	//Dictionary::CreateFromDic(argv[1],argv[2]);
	return 0;
	/*DIR* videodb;   
	struct dirent *p;   
	FILE *pFile;
	int feaid = atoi(argv[4]);	
	char videofile[1024];
	
	if(argc < 5)
	{
		printf("Para Error!\n");
        // 1:videopath, 视频数据存放路径 2:smppath, 特征存放路径; 3:absvideopath, 摘要视频存放路径; 4:startno, -1不对视频名称进行重新编号
		printf("Use: getfea videopath smppath absvideopath startno\n"); 
	}
	
	videodb=opendir(argv[1]);   

	if(videodb == NULL)
	{
		printf("Can not open videodir %s\n",argv[1]);
		return -1;
	}

	long long duration = 0;
	int jpgnum = 20;
	GetFeatureInit();
	// VL_PRINT("vlfeat test\n");
	while ((p=readdir(videodb)))   
	{   
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))   
			continue;   
		else  
		{   
			memset(videofile,0,1024);
			sprintf(videofile,"%s\%s",argv[1],p->d_name);    
			// while(1)
			{
				printf("Getting %s's Feature\n",videofile);
				if(feaid != -1)
				{
					getfeature2(videofile, argv[2],argv[3],feaid++, &duration, jpgnum);
					//getfeature(videofile,argv[2],argv[3],feaid++);
				}
				else
				{
					getfeature2(videofile, argv[2],argv[3],feaid, &duration, jpgnum);
					//getfeature(videofile,argv[2],argv[3],feaid);
				}
				printf("================================Duration is %I64d s\n", duration);
			}
		}
	}
	GetFeatureFinish();
		
	closedir(videodb);
*/
}
