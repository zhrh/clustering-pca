#ifndef PCA_TRANSFORM_H
#define PCA_TRANSFORM_H

#pragma once

#include<vector>
#include<string>
extern "C" {
#include<yael/matrix.h>
#include<yael/vector.h>
}

class PCATransform
{
public:
	static bool Create(const char *dir,int dim,int points,float *&pca_mean,float *&pca_proj);
	static bool Save(const char *fname,const int d,const float *pca_mean,const float *pca_proj);

	// PCATransform(int fnum);		// 初始化const 或引用类型数据成员的唯一机会在构造函数初始化列表
	static bool CreateOnline(const char *dir,int dim,int points,float *&pca_mean,float *&pca_proj);

private:
	static void PerformPCA(int dim,const float *desc,int desc_num,float *pca_mean,float *pca_proj);
	static bool ReadVlad(const char *fname,float *&desc,int &desc_num);		// for test
	 
	static bool CreateBlock(const char *dir,int dim,int points,std::vector<std::string> &filepath,pca_online_t *pca);
	
	//pca_online_t *pca_;
	static int block_num_;
	static const int kBlockFileNum_ = 10000;
};

#endif
