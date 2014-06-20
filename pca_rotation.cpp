#include "pca_rotation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include "dictionary.h"
#include "global.h"


bool PCARotation::CreatePCARotation(const char *dir, int dim, int points, int niter, float *&rotation)
{
	if(dir == NULL || dim <= 0|| points <= 0 || niter <= 0)
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

	float *subset_desc;
	int subset_num;
	if(!Dictionary::DetermineSubset(filepath,dim,points,subset_desc,subset_num))
	{
		printf("Determine subset descriptors failed!\n");
		delete []subset_desc;
		return false;
	}
	printf("%d's points random choosed\n",subset_num);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	printf("calculating ITQ rotation matrix...\n");
	gettimeofday(&start,NULL);
	ITQ(niter, dim, subset_desc, subset_num, rotation);
	gettimeofday(&end,NULL);
	time_used = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	printf("elapsed: %d ms\n",time_used);

	delete []subset_desc;

	return true;
}

bool PCARotation::ITQ(const int niter, const int dim, float *desc, const int desc_num, float *&rotation)
{
	cv::theRNG().state = cv::getTickCount();
	cv::Mat mean = cv::Mat::zeros(1, 1, CV_32FC1);
	cv::Mat sigma = cv::Mat::ones(1, 1, CV_32FC1);
	cv::Mat R(dim, dim, CV_32FC1);;
	cv::randn(R, mean, sigma);
	cv::Mat U, S, VT;
	cv::SVD::compute(R, S, U, VT, cv::SVD::FULL_UV | cv::SVD::MODIFY_A);
	U.copyTo(R);
	cv::Mat V(desc_num, dim, CV_32FC1, desc);
	cv::Mat VR(desc_num, dim, CV_32FC1);
	cv::Mat BT_V(dim, dim, CV_32FC1);
	for(int i = 0; i < niter; ++i)
	{
		printf("iter = %d...\n", i);
		VR = V * R;
		cv::Mat B = cv::Mat::ones(desc_num, dim, CV_32FC1);
		B = B * -1;
		//cv::Mat mask(VR >= 0);
		B.setTo(1, VR >= 0); // here has problems!!!
		BT_V = B.t() * V;
		cv::SVD::compute(BT_V, S, U, VT, cv::SVD::FULL_UV | cv::SVD::MODIFY_A);
		R = VT.t() * U.t();
	}
	//rotation = R.data;
	rotation = (float *)calloc(dim * dim, sizeof(float));
	memcpy(rotation, R.data, dim * dim * sizeof(float));
}

// yael version, the svd function is limited, 返回奇异值个数最多只能是数据维数的一半
/*
bool PCARotation::ITQ(const int niter, const int dim, const float *desc, const int desc_num, float *&rotation)
{
	float *R = fmat_new_rand_gauss(dim, dim);
	float *U = fmat_new_0(dim, dim);
	float *V = fmat_new_0(dim, dim);
	float *S = NULL;
	fmat_svd_partial(dim, dim, dim / 2, R, S, U, V);
	memcpy(R, U, sizeof(float) * dim * dim);
	float *B =  fmat_new_0(desc_num, dim);
	float *VR = fmat_new_0(desc_num, dim);
	float *BT_V = fmat_new_0(dim, dim);
	for(int i = 0; i < niter; ++i)
	{
		// VR = V * R
		fmat_mul_tl(desc, R, desc_num, dim, dim, VR);
		// B = sign(VR)
		for(int j = 0; j != dim; ++j)
			for(int k = 0; k != desc_num; ++k)
			{
				if(VR[j * desc_num + k] < 0)
					B[j * desc_num + k] = -1;
				else
					B[j * desc_num + k] = 1;
			}
		// B'V
		fmat_mul_tlr(B, desc, dim, desc_num, dim, BT_V);
		// [S,U,V] = svd(B'V)
		fmat_svd_partial(dim, dim, dim / 2, BT_V, S, U, V);
		// R = VU'
		fmat_mul_tr(V, U, dim, dim, dim, R);
	}
	free(B);
	free(U);
	free(V);
	free(VR);
	free(BT_V);
	rotation = R;
}
*/

bool PCARotation::SavePCARotation(const char *fname, const int dim, const float *rotation)
{
	FILE *fid = fopen(fname,"wb");
	if(!fid)
	{
		printf("can't open the file %s\n",fname);
		return false;
	}
	fvec_fwrite(fid, rotation, dim * dim);
	fclose(fid);
	return true;
}



