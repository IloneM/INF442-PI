#include "trainer.h"
#include <random>
//#include <filesystem>
//#include <ifstream>

//class RootTrainer {
void RootTrainer::init() {
//	if(initialized) return;

	MPI_Comm_size(MPI_COMM_WORLD, &nbworkers);

	Grid tl({{0,0},{width()-MIN_SIZE,height()-MIN_SIZE}}, STEP, STEP);

	/* size when both grids have same same xstep and ystep. See report for proof */
	unsigned sizeperworker = tl.cardwidth() * (tl.cardwidth()+1)/2
							* tl.cardheight() * (tl.cardheight()+1)/2
							* NB_DIFF_FEATURES / (size-1) + 1;
	MPI_Bcast(&sizeperworker, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	int workersit;

	for(Grid::iterator tlit=tl.begin(); tlit < tl.end(); tlit++) {
		Grid br({{tlit->x + MIN_SIZE, tlit->y + MIN_SIZE},{width(),height()}}, STEP, STEP);
		for(Grid::iterator brit = br.begin(); brit < br.end(); brit++) {
			Rect instruct = {*tlit, *brit};
			MPI_Send(&instruct, sizeof(Rect), MPI_BYTE, workersit+1, 1, MPI_COMM_WORLD);

			workersit = (workersit+1)%(nbworkers-1);
			nbfeatures++;
		}
	}

	for(workersit=1; i< nbworkers; workersit++)
		MPI_Send(NULL, 0, MPI_BYTE, i, 0, MPI_COMM_WORLD);

	nbfeatures *= NB_DIFF_FEATURES;
//	initialized = true;

	MPI_Barrier(MPI_COMM_WORLD);
}

void WorkersTrainer::init() {
//	if(initialized) return;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	unsigned workertargetssize;
	MPI_Bcast(&workertargetssize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	workertargets.reserve(workertargetssize);

	Rect instruct;
	do {
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(status.MPI_TAG == 1) {
			MPI_Recv(&instruct, sizeof(Rect), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			workertargets.push_back(instruct);
		} else if(status.MPI_TAG == 0) {
			MPI_Recv(NULL, 0, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} 
	} while(status.MPI_TAG);

//	initialized = true;
//	workerfeatures.reserve(workertargets.size() * NB_DIFF_FEATURES);
	workerclassifiers.resize(workertargets.size() * NB_DIFF_FEATURES, Perceptron(EPS));
	workerfeatures.resize(_K);

	MPI_Barrier(MPI_COMM_WORLD);
}

Features* RootTrainer::computeFeatures(bool returnsthg) {
//	if(!initialized) init();

	uint8_t returnsthgbuff = (uint8_t)returnsthg;
	MPI_Bcast(&returnsthgbuff, 1, MPI_BYTE, 0, MPI_COMM_WORLD);

	Features* result = NULL;
	if(returnsthg) {
		result = new Features(nbfeatures);
		Feature buffer;
		for(int i=0; i< nbfeatures; i++) {
			MPI_Recv(&buffer, 1, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			result->emplace(getKey(buffer), getValue(buffer));
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	return result;
}

Features* WorkersTrainer::computeFeatures(unsigned imgID, featclass_t fc) {
//	if(!initialized) init();

	uint8_t returnsthg;
	MPI_Bcast(&returnsthg, 1, MPI_BYTE, 0, MPI_COMM_WORLD);

	Feature buffer;
	std::pair<FeatureType, pixel> features[NB_DIFF_FEATURES];
	std::vector<Feature> imgfeatures(workertargets.size() * NB_DIFF_FEATURES);

	for(int i=0; i< workertargets.size(); i++) {
		getPos(buffer) = workertargets[i];
		integral->computeFeaturesOn(getPos(buffer), features);
		for(int j=0; j< NB_DIFF_FEATURES; j++) {
			getFt(buffer) = features[i].first;
			getValue(buffer) = features[i].second;
			imgfeatures[i*NB_DIFF_FEATURES + j] = buffer;
			if(returnsthg)
				MPI_Send(&buffer, 1, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		}
	}
	workerfeatures.push_back(imgfeatures);

	MPI_Barrier(MPI_COMM_WORLD);
	return NULL;
}

void start(int _K) {
	K = _K;

//	if(!initialized) init();
	
	int choice;
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution();

	std::string filepath;

	//	Trainer() : classifierstorage(), featuresstorage(NB_NEG+NB_POS, NULL), 
//		Classifiers* classifierstorage = new Classifiers();
	std::uniform_int_distribution<int> distribution(0,NB_NEG+NB_POS-1);
	for(unsigned int i=0; i< K; i++) {
		choice =  distribution(generator);

		MPI_Bcast(choice, 1, MPI_INT, 0, MPI_COMM_WORLD);

		std::string filepath;
		if(choice > NB_NEG) {
			filepath = POS_PREFIX + std::to_string(choice-NB_NEG) + IMG_SUFFIX;
		} else {
			filepath = NEG_PREFIX + std::to_string(choice) + IMG_SUFFIX;
		}

		Features* features = PMImage(filepath).features();
	}
}

static Classifiers* Trainer::train(const std::string& path, const unsigned int& K, const weights_t& eps=EPS) {
	int rank;
	int size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	//Classifiers* classifierstorage = NULL;
	//std::vector<Features*> featuresstorage;
	std::string negprefix = path + (path[path.size()-1]=='/'?"neg/im":"/neg/im");
	std::string posprefix = path + (path[path.size()-1]=='/'?"pos/im":"/pos/im");

	int choice;
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution();

	//	Trainer() : classifierstorage(), featuresstorage(NB_NEG+NB_POS, NULL), 
//		Classifiers* classifierstorage = new Classifiers();
	if(!rank) {
		std::uniform_int_distribution<int> distribution(0,NB_NEG+NB_POS-1);
	}
	for(unsigned int i=0; i< K; i++) {
		if(!rank) {
			choice =  distribution(generator);
		}
		MPI_Bcast(choice, 1, MPI_INT, 0, MPI_COMM_WORLD);

		std::string filepath;
		if(choice > NB_NEG) {
			filepath = posprefix + std::to_string(choice-NB_NEG) + IMG_SUFFIX;
		} else {
			filepath = negprefix + std::to_string(choice) + IMG_SUFFIX;
		}

		Features* features = PMImage(filepath).features();
	}
}

