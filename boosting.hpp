#ifndef IMAGE_H
#define IMAGE_H

#define NUMB_IMAGE

#include <mpi.h>
#include <CImg.h>
#include "feature.h"
#include "perceptron.h"
#include <utility>
#include <vector>

struct boosting {
	weights_t wa; // aka wi1
	weights_t wb;
	double alpha;
}