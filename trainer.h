#ifndef TRAINER_H
#define TRAINER_H

#include "feature.h"
#include "integralimg.h"
#include "perceptron.h"
#include <string>
#include <mpi.h>

#define EPS 1e-6

/* better: use filesystem */
#define NB_NEG 4415
#define NB_POS 818

#define TRAIN_DIR(path) "./app/" path

#define IMG_SUFFIX ".jpg"
#define NEG_PREFIX TRAIN_DIR("neg/im")
#define POS_PREFIX TRAIN_DIR("pos/im")

#define IMG_DIMS {112,92}

typedef std::unordered_map<FeatureKey, Perceptron> Classifiers;

class Trainer {
public:
	virtual void start(int K)=0;
};

class RootTrainer : public Trainer {
public:
	RootTrainer() : /*initialized(false),*/ nbfeatures(0) {}

	virtual void start(int K);

	Features* uselessComputeFeatures() { computeFeatures(true); }
protected:
	virtual void init();

	virtual Features* computeFeatures(bool returnsthg = false);

//	bool initialized;
	int nbworkers;
	unsigned nbfeatures;
};

class WorkersTrainer : public Trainer {
public:
	WorkersTrainer(Image* _integral) : workerclassifiers(0), workertargets(0), workerfeatures(0), integral(_integral) {}
//										/*initialized(false)*/ integral(NULL) {}

	virtual void start(int K);

protected:
	virtual void init();

	virtual Features* computeFeatures(unsigned imgID);

	int rank;
//	bool initialized;

	std::vector<Rect> workertargets;
	std::vector<std::vector<Feature>> workerfeatures;
	std::vector<Perceptron> workerclassifiers;
	Image* integral;
};

//class Trainer {
//public:
//	Trainer() : classifierstorage(), featuresstorage(NB_NEG+NB_POS, NULL), 

//	static Classifiers* train(const std::string& path, const unsigned int& K, const weights_t& eps=EPS)
//protected:
//	void trainstep(const Features* features);
	
//	std::vector<Features*> featuresstorage;
//	Classifiers classifierstorage;
//};

#endif
