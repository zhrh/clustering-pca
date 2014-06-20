#include"pca_transform.h"
#include<dirent.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string.h>
#include<sys/time.h>
#include<unistd.h>
#include<algorithm>
#include"dictionary.h"
#include"global.h"
#include"utils.h"


bool PCATransform::Create(const char *dir,int dim,int points,float *&pca_mean,float *&pca_proj)
{
	if(dir == NULL || dim <= 0|| points <= 0)
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
	Dictionary::ReadSurfPath(dir,filepath);
	if(filepath.empty())
	{
		printf("Don't find any features file!\n");
		return false;
	}
	printf("%d featurs file find\n",filepath.size());
	printf("randomizing image list...\n");
	random_shuffle(filepath.begin(),filepath.end());
	if(filepath.size() > kMaxFileNum)
	{
		printf("Reach the max file number...\n");
		filepath.erase(filepath.begin() + kMaxFileNum, filepath.end());
		printf("The file number change to: %d\n",filepath.size());
	}
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);
	
	printf("creating subset of points...\n");
	gettimeofday(&start,NULL);
	float *subset_desc;
	int subset_num;
	if(!Dictionary::DetermineSubset(filepath,dim,points,subset_desc,subset_num))
	//if(!ReadVlad(dir,subset_desc,subset_num))
	{
		printf("Determine subset descriptors failed!\n");
		delete []subset_desc;
		return false;
	}
	printf("%d's points random choosed\n",subset_num);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	printf("calculating pca matrix...\n");
	gettimeofday(&start,NULL);
	pca_mean = new float[dim]();
	pca_proj = new float[dim * dim]();
	PerformPCA(dim,subset_desc,subset_num,pca_mean,pca_proj);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);
	delete []subset_desc;
	//free(subset_desc);	// for test
	return true;

}

bool PCATransform::CreateFromVlad(const char *dir, int dim, float *&pca_mean,float *&pca_proj)
{
	struct timeval start,end;
	int time_used;
	gettimeofday(&start,NULL);
	printf("randomizing file list...\n");
	std::vector<int> fileid;
	//const int kVladFileNum = 50000000;
	const int kVladFileNum = 1000000;
	const int kInitId = 5000000;
	for(int i = kInitId; i != kVladFileNum; ++i)
		fileid.push_back(i);
	random_shuffle(fileid.begin(),fileid.end());
	if(fileid.size() > kMaxFileNum)
	{
		printf("Reach the max file number...\n");
		fileid.erase(fileid.begin() + kMaxFileNum, fileid.end());
	}
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	printf("Load vlad file...\n");
	gettimeofday(&start,NULL);
	int split_num = 100000;
	char filepath[1024];
	float *desc;
	int *frameid = NULL;
	int desc_num = 0, subset_num = 0;
	float *subset_desc = new float[kMaxFileNum * dim](); 	
	if(subset_desc == NULL)
	{
		fprintf(stderr, "Can't apply such memory!\n");
		return false;
	}
	for(std::vector<int>::const_iterator iter = fileid.begin(); iter != fileid.end(); ++iter)
	{
		int id = *iter;
		int subdirnum = ( id / split_num + 1) * split_num;
		memset(filepath, 0, 1024);
		sprintf(filepath,"%s/%d/%d.vlad", dir, subdirnum, 1000000000 + id);
		if(LoadVlad(filepath, desc_num, desc, frameid) < 0)
		{
			printf("Can't load vlad descriptor from %s\n",filepath);
			continue;
		}
		memcpy(subset_desc + subset_num * dim, desc, dim * sizeof(float));
		++subset_num;
		free(desc);
	}
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	printf("calculating pca matrix...\n");
	gettimeofday(&start,NULL);
	pca_mean = new float[dim]();
	pca_proj = new float[dim * dim]();
	PerformPCA(dim,subset_desc,subset_num,pca_mean,pca_proj);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);
	delete []subset_desc;
	//free(subset_desc);	// for test
	return true;
}


bool PCATransform::CreateFromFvecs(const char *fname,int dim,int points,float *&pca_mean,float *&pca_proj)
{
	struct timeval start,end;
	int time_used;
	printf("Read fvecs of points...\n");
	gettimeofday(&start,NULL);
	float *subset_desc = new float[dim * points]();
	int subset_num;
	//printf("dim = %d, points = %d\n",dim ,points);
	subset_num = fvecs_read(fname, dim, points, subset_desc);
	printf("%d's points random choosed\n",subset_num);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	printf("calculating pca matrix...\n");
	gettimeofday(&start,NULL);
	pca_mean = new float[dim]();
	pca_proj = new float[dim * dim]();
	PerformPCA(dim,subset_desc,subset_num,pca_mean,pca_proj);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);
	delete []subset_desc;
	//free(subset_desc);	// for test
	return true;
}

void PCATransform::PerformPCA(int dim,const float *desc,int desc_num,float *pca_mean,float *pca_proj)
{
	struct timeval start,end;
	int time_used;
	int assume_centered = 0;
	printf("\tcalculating covariance...\n");
	gettimeofday(&start,NULL);
	float *cov = fmat_new_covariance(dim,desc_num,desc,pca_mean,assume_centered);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("\telapsed: %d ms\n",time_used);

	float *evals = new float[dim]();
	printf("\tcalculating eigenvectors...\n");
	gettimeofday(&start,NULL);
	float *pcamat = fmat_new_pca_from_covariance(dim,cov,evals);

	// add by zrh140411
	FILE *fid = fopen("./result/singlevals.fvec","wb");
	if(fid == NULL)
	{
		fprintf(stderr, "can't open the file singlevals.fvec\n");
		return;
	}
	fvec_fwrite(fid,evals,dim);
	fclose(fid);

	//for(int i = 0;i != dim;++i)
	//	for(int j = 0;j != dim;++j)
	//		pca_proj[i * dim + j] = pcamat[i * dim + j];
	memcpy(pca_proj,pcamat,sizeof(float) * dim * dim);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("\telapsed: %d ms\n",time_used);
	free(cov);
	free(evals);
	free(pcamat);
}

bool PCATransform::Save(const char *fname,const int d,const float *pca_mean,const float *pca_proj)
{
	FILE *fid = fopen(fname,"wb");
	if(!fid)
	{
		printf("can't open the file %s\n",fname);
		return false;
	}
	fvec_fwrite(fid,pca_mean,d);
	fvec_fwrite(fid,pca_proj,d * d);
	fclose(fid);
	return true;
}

bool PCATransform::ReadVlad(const char *fname,float *&desc,int &desc_num)		// for test
{
	int dim;
	desc_num = fvecs_new_read(fname,&dim,&desc);
	return true;
}

const int PCATransform::kBlockFileNum_;
int PCATransform::block_num_ = 0;
bool PCATransform::CreateOnline(const char *dir,int dim,int points,float *&pca_mean,float *&pca_proj)
{	
	if(dir == NULL || dim <= 0|| points <= 0)
	{
		printf("invalid parameters\n");
		return false;
	}
	std::vector<std::string> filepath;
	filepath.clear();
	pca_online_t *pca = pca_online_new(dim);
	CreateBlock(dir,dim,points,filepath,pca);
	// for rest file
	if(!filepath.empty())
	{
		++block_num_;
		struct timeval start,end;
		int time_used;
		printf("Online, Block %d........\n",block_num_);
		printf("randomizing image list...\n");
		random_shuffle(filepath.begin(),filepath.end());

		printf("creating subset of points...\n");
		gettimeofday(&start,NULL);
		float *subset_desc;
		int subset_num;
		if(!Dictionary::DetermineSubset(filepath,dim,points,subset_desc,subset_num))	// 采用这种方式就没有必要subset了
		{
			printf("Determine subset descriptors failed!\n");
			delete []subset_desc;
			return false;
		}
		printf("%d's points random choosed\n",subset_num);
		gettimeofday(&end,NULL);
		time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
		printf("elapsed: %d ms\n",time_used);
		
		printf("calculating pca matrix...\n");
		gettimeofday(&start,NULL);
		float *subset_t = fmat_new_transp(subset_desc,dim,subset_num); // need transposition
		pca_online_accu(pca,subset_t,subset_num);	// 感觉计算mean有点问题,zrh1219???		
		pca_online_complete(pca);
		gettimeofday(&end,NULL);
		time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
		printf("elapsed: %d ms\n",time_used);
		delete []subset_desc;
		delete []subset_t;
		filepath.clear();
	}
	pca_mean = new float[dim]();
	pca_proj = new float[dim * dim]();
	for(int i = 0;i != dim;++i)
	{
		pca_mean[i] = pca->mu[i];
		for(int j = 0;j != dim;++j)
			pca_proj[i * dim + j] = pca->eigvec[i * dim + j];
	}

	pca_online_delete(pca);
	return true;
}

bool PCATransform::CreateBlock(const char *dirname,int dim,int points,std::vector<std::string> &filepath,pca_online_t *pca)
{
	DIR *dir;
	struct dirent *file; 	//readdir函数的返回值就存放在这个结构体中
	struct stat st;

	if(!(dir = opendir(dirname)))
	{
		printf("error opendir %s!!!\n",dirname);
		return false;
	}
	while((file = readdir(dir)) != NULL)
	{
		if((strcmp(file->d_name,".") == 0) || (strcmp(file->d_name,"..") == 0))
      		continue;

		char tmp_path[1024];
		memset(tmp_path,0,1024);
		sprintf(tmp_path,"%s/%s",dirname,file->d_name);
		printf("%s\n",tmp_path);
		if(stat(tmp_path,&st) >= 0 && S_ISDIR(st.st_mode))
		{
			CreateBlock(tmp_path,dim,points,filepath,pca);	// 子目录继续递归
		}
		else
		{
			char *sub = strrchr(tmp_path,'.');
			//char surfix[10];
			//memset(surfix,0,10);
        	//strncpy(surfix,sub + 1, strlen(sub) - 1);
			if (!strcmp(sub + 1,"surf") || !strcmp(sub + 1,"siftgeo")) // for test sift features
			{
				std::string path(tmp_path);
				filepath.push_back(path);
				if(filepath.size() == kBlockFileNum_)
				{	
					++block_num_;
					struct timeval start,end;
					int time_used;
					printf("Online, Block %d........\n",block_num_);
					printf("randomizing image list...\n");
					random_shuffle(filepath.begin(),filepath.end());

					printf("creating subset of points...\n");
					gettimeofday(&start,NULL);
					float *subset_desc;
					int subset_num;
					if(!Dictionary::DetermineSubset(filepath,dim,points,subset_desc,subset_num))	// 采用这种方式就没有必要subset了
					{
						printf("Determine subset descriptors failed!\n");
						delete []subset_desc;
						return false;
					}
					printf("%d's points random choosed\n",subset_num);
					gettimeofday(&end,NULL);
					time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
					printf("elapsed: %d ms\n",time_used);
					
					printf("calculating pca matrix...\n");
					gettimeofday(&start,NULL);
					float *subset_t = fmat_new_transp(subset_desc,dim,subset_num); // need transposition???
					pca_online_accu(pca,subset_t,subset_num);
					pca_online_complete(pca);
					gettimeofday(&end,NULL);
					time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
					printf("elapsed: %d ms\n\n",time_used);
					delete []subset_desc;
					delete []subset_t;
					filepath.clear();
				}	
			}
		}
	}
	closedir(dir);
	return true;
}
