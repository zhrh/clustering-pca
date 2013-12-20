#ifndef GET_CENTROIDS_H
#define GET_CENTROIDS_H

#pragma once

#include<vector>
#include<string>

class Dictionary
{
public:
	static bool Create(const char *dir,int clusters,int dim,int points,float *&centroids);
	static void CreateFromDic(const char *dic_fname, const char *cen_fname);
	static void Save(const char *fname,const int d,const int n,const float *centroids);

	// read surf features from surf directory and its subdirectories
	static bool ReadSurfPath(const char *surfdir,std::vector<std::string> &filepath);
	// determine subset of interest points
	static bool DetermineSubset(const std::vector<std::string> &filepath,int dim,int points,float *&subset,int &subsetp);

private:
	// extract random points
	static void ExtractRandomPoints(int dim,int points,const float *src,int &desc_num,float *dst);
	// perform the clustering
	static bool PerformClustering(int dim,int clusters,int niter,int redo,const float *desc,int desc_num,float *centroids);
	static bool ReadDic(const char *fname, int d, int n, float *v);
};
#endif
