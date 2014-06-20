#ifndef PCA_ROTATION_H
#define PCA_ROTATION_H

#pragma once

extern "C" {
#include<yael/vector.h>
}


class PCARotation
{
public:
	static bool CreatePCARotation(const char *dir, int dim, int points, int niter, float *&rotation);
	static bool SavePCARotation(const char *fname, const int dim, const float *rotation);

private:
	static bool ITQ(const int niter, const int dim, float *desc, const int desc_num, float *&rotation);
	
};

#endif
