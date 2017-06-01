#include "image.h"

Image* Image::integral(Image* input, Image* output) {
	if(!output or output->width()<=0 or output->height()<=0
		or !input or input->width() != output->width() or input->height() != output->height())
		return NULL;

	int width = input->width();
	int height = input->height();

	pixel* res = output->data();
	pixel* raw = input->data();

	res[0] = raw[0];
	for(int i=1; i< width; i++) {
		res[i] = res[i-1] + raw[i];
	}
	for(int j=1; j< height; j++) {
		res[j*width] = res[(j-1)*width] + raw[j*width];
	}
	pixel s;
	for(int i=1; i< width; i++) {
		s = res[i];
		for(int j=1; j< height; j++) {
			s += raw[i+j*width];
			res[i+j*width] = s + res[i-1+j*width];
		}
	}

	return output;
}

Image* Image::integral(Image* output) {
	if(output && width()>0 && height()>0 && output->width() == width() && output->height() == height())
		return Image::integral(this, output);
	return NULL;
}

//Features* Image::features(Image* integralbuffer) {
//Features* features(Image* integralbuffer, std::vector<Rect>& workertargets) {
Features* features(Image* integralbuffer);
	if(!integral(integralbuffer))
		return NULL;

	int rank;
	Features* result = NULL;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(!rank) {
		Grid tl({{0,0},{width()-MIN_SIZE,height()-MIN_SIZE}}, STEP, STEP);

		result = new Features( /* size when both grids have same same xstep and ystep. See report for proof */
					tl.cardwidth() * (tl.cardwidth()+1)/2
		            * tl.cardheight() * (tl.cardheight()+1)/2
					* NB_DIFF_FEATURES
				 );

		int size;
		MPI_Comm_size(MPI_COMM_WORLD, &size);

		if(size > 1 and useowntargets and !workertargetsinitialized) {
			unsigned sizeperworker = result->size() / (size-1);
			MPI_Bcast(&sizeperworker, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		}

		Feature buffer;

		for(Grid::iterator tlit=tl.begin(); tlit < tl.end(); tlit++) {
			Grid br({{tlit->x + MIN_SIZE, tlit->y + MIN_SIZE},{width(),height()}}, STEP, STEP);

			for(Grid::iterator brit = br.begin(); brit < br.end(); brit++) {
				getPos(buffer) = {*tlit, *brit};
				if(size > 1) {
					if(!workertargetsinitialized) {
						MPI_Status status;
						do {
							MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
							if(status.MPI_TAG == 0) {
								for(int i=0; i< NB_DIFF_FEATURES; i++) {
									MPI_Recv(&buffer, sizeof(Feature), MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD,
											 MPI_STATUS_IGNORE);
									result->emplace(getKey(buffer), getValue(buffer));
								}
							} else if(status.MPI_TAG == 1) {
								MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
								Rect& instruct = getPos(buffer);
								MPI_Send(&instruct, sizeof(Rect), MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
							}
						} while(status.MPI_TAG);
					} else {
						for(int i=0; i< NB_DIFF_FEATURES; i++) {
							MPI_Recv(&buffer, sizeof(Feature), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
									 MPI_STATUS_IGNORE);
							result->emplace(getKey(buffer), getValue(buffer));
						}
					}
				} else {
					std::pair<FeatureType, pixel> features[NB_DIFF_FEATURES];
					computeFeaturesOn(getPos(buffer), integralbuffer, features);

					for(int i=0; i< NB_DIFF_FEATURES; i++) {
						getFt(buffer) = features[i].first;
						getValue(buffer) = features[i].second;
						result->emplace(getKey(buffer), getValue(buffer));
					}
				}
			}
		}
		if(size > 1 and !workertargetsinitialized) {
			for(int i=1; i< size; i++) {
				MPI_Status status;
				do {
					MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
					if(status.MPI_TAG == 0) {
						for(int i=0; i< NB_DIFF_FEATURES; i++) {
							MPI_Recv(&buffer, sizeof(Feature), MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD,
									 MPI_STATUS_IGNORE);
							result->emplace(getKey(buffer), getValue(buffer));
						}
					} else if(status.MPI_TAG == 1) {
						MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						MPI_Send(NULL, 0, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
					}
				} while(status.MPI_TAG != 1);
			}
			workertargetsinitialized = useowntargets;
		}
	} else {
		Feature buffer;
		std::pair<FeatureType, pixel> features[NB_DIFF_FEATURES];

		if(!workertargetsinitialized) {
			if(useowntargets) {
				unsigned workertargetssize;
				MPI_Bcast(&workertargetssize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
				workertargets = new std::vector<Rect>(workertargets);
			}

			MPI_Status status;
			do {
				MPI_Send(NULL, 0, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
				MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				if(status.MPI_TAG == 0) {
					MPI_Recv(NULL, 0, MPI_CHAR,
							 status.MPI_SOURCE, 0, MPI_COMM_WORLD,
							 MPI_STATUS_IGNORE);
				} if(status.MPI_TAG == 1) {
					Rect& instruct = getPos(buffer);
					MPI_Recv(&instruct, sizeof(Rect), MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					computeFeaturesOn(instruct, integralbuffer, features);
					for(int i=0; i< NB_DIFF_FEATURES; i++) {
						getFt(buffer) = features[i].first;
						getValue(buffer) = features[i].second;
						MPI_Send(&buffer, sizeof(Feature), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
					}
					if(useowntargets) workertargets.push_back(instruct);
				} 
			} while(status.MPI_TAG);
		} else {
			for(int i=0; i< workertargets.size(); i++) {
				Rect& instruct = getPos(buffer) = workertargets->at(i);
				computeFeaturesOn(instruct, integralbuffer, features);
				for(int i=0; i< NB_DIFF_FEATURES; i++) {
					getFt(buffer) = features[i].first;
					getValue(buffer) = features[i].second;
					MPI_Send(&buffer, sizeof(Feature), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
				}
			}
		}
	}
	return result;
}

void Image::computeFeaturesOn(const Rect& pos, const Image* integralofthis, std::pair<FeatureType, unsigned int>* output) {
	using namespace Rectfuncs;
	uint32_t xm, ym;

	//case FeatureType::vsplit
	xm = xfraction(pos);
	output[0] = std::make_pair( FeatureType::vsplit,
				integralofthis->at({xm, bottom(pos)}) - integralofthis->at({xm, top(pos)})
			  - integralofthis->at(bottomRight(pos))  + integralofthis->at(topRight(pos)));

	//case FeatureType::hsplit
	ym = yfraction(pos);
	output[1] = std::make_pair( FeatureType::hsplit,
				integralofthis->at(bottomRight(pos)) - integralofthis->at(bottomLeft(pos))
			  - integralofthis->at({right(pos), ym}) + integralofthis->at({left(pos), ym}));

	//case FeatureType::cantor
	xm = xfraction(pos);
	ym = xfraction(pos, 3, 2);
	output[2] = std::make_pair( FeatureType::cantor,
				integralofthis->at(bottomRight(pos)) - integralofthis->at(topRight(pos))
			  - integralofthis->at({ym, bottom(pos)}) + integralofthis->at({ym, top(pos)})
			  + integralofthis->at({xm, bottom(pos)}) + integralofthis->at(topLeft(pos))
			  - integralofthis->at({xm, top(pos)}) - integralofthis->at(bottomLeft(pos)));

	//case FeatureType::checked
	xm = xfraction(pos);
	ym = yfraction(pos);
	output[3] = std::make_pair( FeatureType::checked,
				2*integralofthis->at({xm, ym}) - integralofthis->at({xm, top(pos)})
			  - 2*integralofthis->at({right(pos),ym})  + integralofthis->at(topRight(pos))
			  + integralofthis->at(bottomRight(pos)) //- integralofthis->at({right(pos), ym})
			  - integralofthis->at({xm, bottom(pos)}));// + integralofthis->at({xm, ym})
}

PMImage::PMImage(const pixel* dat, const int& w, const int& h) : Pixelmap(dat, w, h), integralbuffer(NULL) {}
PMImage::PMImage(const int& w, const int& h) : Pixelmap(w, h), integralbuffer(NULL) {}
PMImage::PMImage(const char* const filename) : Pixelmap(filename), integralbuffer(NULL) {}
PMImage::PMImage(const PMImage& other) : Pixelmap(other), integralbuffer(NULL) {}
PMImage::PMImage(const Pixelmap& other) : Pixelmap(other), integralbuffer(NULL) {}

PMImage::~PMImage() {
	if(integralbuffer) delete integralbuffer;
}

inline PMImage* PMImage::integral() {
	return (PMImage*) Image::integral(init_integralbuffer());
}

PMImage* PMImage::init_integralbuffer() {
	if(!integralbuffer)
		integralbuffer = new PMImage(width(), height());
	return integralbuffer;
}

