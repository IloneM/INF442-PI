#include "boosting.hpp"
#include "math.h"


int errorFunction(Perceptron p, Feature f , int c){
	if( p(getValue(f)) == c )
		return 0;
	else
		return 1;
}


vector<boosting> finalClassifier(Image** allImages){

	vector<boosting> boostingFunction;

	//Features* features(Image* integralbuffer);

	double *poids = new double[NUMB_IMAGE];
	for(unsigned int i=0; i<NUMB_IMAGE; ++i){
		poids[i] = 1/NUMB_IMAGE;
	}

	vector<Perceptron> classifier;
	vector<PerceptronWeights> classifierWeights;
	vector<uint8_t> imageClass;
	vector<vector<Feature>> vec;

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank!=0){
		double *err = new double[vec.size()/(size-1)];
		int l;
		double sum;
		int indMin;
		double errMin;
	}else{
		double alpha;
		double *error = new double[size];
		int *errorIndex = new int[size];
		int indexMin;
		double errorMin;
		double normalize;
	}



	for(int k=0; k<N; k++){
		if(rank!=0){
			l=0;
			for(int i = (rank-1) * (vec.size()/(size-1)); i < rank * (vec.size()/(size-1)); ++i){
				sum = 0;
				for(int j = 0; j < NUMB_IMAGE; ++j){
					sum += poids[j] * errorFunction(classifier[i], vec[i][j], imageClass[j]);
				}
				err[l] = sum;
				l++;
			}
			indMin = 0;
			errMin = err[0]
			for(int i = 1; i < vec.size()/(size-1); ++i){
				if(err[i] < errMin){
					indMin = i;
					errMin = err[i];
				}	
			}
			indMin += (rank-1) * (vec.size()/(size-1));
			MPI_Gather(errMin, 1, double, NULL, 1, double, 0, MPI_COMM_WORLD);
			MPI_Gather(indMin, 1, int, NULL, 1, int, 0, MPI_COMM_WORLD);

		}else{
			//que faier des send_data du root ??
			
			MPI_Gather(error, 1, double, error, 1, double, 0, MPI_COMM_WORLD);
			MPI_Gather(errorIndex, 1, int, errorIndex, 1, int, 0, MPI_COMM_WORLD);

			indexMin = errorIndex[1];
			errorMin = error[1]
			for(int i = 2; i < size; ++i){
				if(error[i] < errorMin){
					indexMin = errorIndex[i];
					errorMin = error[i];
				}	
			}
			alpha = 0.5 * log((1-errorMin)/errorMin);
			boostingFunction.push_back({classifierWeights[indexMin].wa, classifierWeights[indexMin].wa, alpha});
			for(int j = 0; j < NUMB_IMAGE; ++j){
				poids[j] = poids[j] * exp(-imageClass[j] * alpha * classifier[indexMin](vec[indexMin][j]));
			}
			normalize = 0;
			for(int j = 0; j < NUMB_IMAGE; ++j){
				normalize += poids[j];
			}
			for(int j = 0; j < NUMB_IMAGE; ++j){
				poids[j] = poids[j]/normalize;
			}

		}


	}









}