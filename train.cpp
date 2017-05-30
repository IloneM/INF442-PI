#include "trainer.h"

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	
	Trainer::train("dev", 1e5);

	MPI_Finalize();

	return 0;
}

