#include "trainer.h"
#include <random>
//#include <filesystem>
//#include <ifstream>

static Classifiers* Trainer::train(const std::string& path, const unsigned int& K, const weights_t& eps=EPS) {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Classifiers* classifierstorage = NULL;
	if(!rank) {
		std::string negprefix = path + (path[path.size()-1]=='/'?"neg/im":"/neg/im");
		std::string posprefix = path + (path[path.size()-1]=='/'?"pos/im":"/pos/im");

		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0,NB_NEG+NB_POS-1);

	//	Trainer() : classifierstorage(), featuresstorage(NB_NEG+NB_POS, NULL), 
	//	std::vector<Features*> featuresstorage;
		Classifiers* classifierstorage = new Classifiers();
	}

	for(unsigned int i=0; i< K; i++) {
		int choice =  distribution(generator);
		std::string filepath;
		if(choice > NB_NEG) {
			filepath = posprefix + std::to_string(choice-NB_NEG) + IMG_SUFFIX;
		} else {
			filepath = negprefix + std::to_string(choice) + IMG_SUFFIX;
		}
		Features* features = PMImage(filepath).features();
	}
}

