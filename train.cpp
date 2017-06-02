#include "trainer.h"

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	
	int K = 100;

	if(argc > 1) K = std::stoi(argv[1]);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Trainer* trainer;

	if(rank) {
		Point dims = IMG_DIMS;
		trainer = new WorkersTrainer(new PMImage(dims.x, dims.y));
	} else {
		trainer = new RootTrainer();
	}

	trainer->start(K);

	MPI_Finalize();

	return 0;
}

