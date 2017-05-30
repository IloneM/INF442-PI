#ifndef TRAINER_H
#define TRAINER_H

#include "feature.h"
#include "image.h"
#include "perceptron.h"
#include <string>
#include <mpi.h>

#define EPS 1e-6

/* better: use filesystem */
#define NB_NEG 8831
#define NB_POS 1637

#define IMG_SUFFIX ".jpg"

typedef std::unordered_map<FeatureKey, Perceptron> Classifiers;

class Trainer {
public:
//	Trainer() : classifierstorage(), featuresstorage(NB_NEG+NB_POS, NULL), 

	static Classifiers* train(const std::string& path, const unsigned int& K, const weights_t& eps=EPS)
protected:
	void trainstep(const Features* features);
	
//	std::vector<Features*> featuresstorage;
//	Classifiers classifierstorage;
};

#endif
