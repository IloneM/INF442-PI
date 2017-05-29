#include <iostream>
#include <fstream>
#include <mpi.h>
#include <CImg.h>
#include "tools.h"
#include "image.h"

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	PMImage testi("app/neg/im0.jpg");
	//testi.display();
	//testi.integral()->display();
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Features* res = testi.features();

	if(!rank)
		for (Features::iterator it= res->begin(); it!=res->end(); ++it) {
			std::cout /*<< it->first*/ << ": " << it->second << '\n';
		}
//	std::cout << '\n';

//	testi.display();
//	testi.integral()->display();

//	Grid test({{0,0},{120,130}}, 4, 3);
////
//	for(Grid::iterator it=test.fromindex(132/3*120/4+120/3); it < test.end(); it++) {
//		std::cout << it->x << ' ' << it->y << '\n';
//	}
//	std::cout << test.end()->x << ' ' << test.end()->y << '\n';


	MPI_Finalize();

	return 0;
}

