#include "trainer.h"
#include <random>
//#include <filesystem>
//#include <ifstream>

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

