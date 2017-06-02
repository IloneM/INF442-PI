#include "integralimg.h"

Image::Image() {
	pixel* dat = data();

	for(int i=1; i< width(); i++) {
		dat[i] += dat[i-1];
	}
	for(int j=1; j< height(); j++) {
		dat[j*width()] += dat[(j-1)*width()];
	}
	pixel s;
	for(int i=1; i< width(); i++) {
		s = dat[i];
		for(int j=1; j< height(); j++) {
			s += dat[i+j*width()];
			dat[i+j*width()] = s + dat[i-1+j*width()];
		}
	}
} 

Image* Image::integral(const Image* input, Image* output) {
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

//Image* Image::integral(Image* output) {
//	if(output && width()>0 && height()>0 && output->width() == width() && output->height() == height())
//		return Image::integral(this, output);
//	return NULL;
//}

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

std::pair<FeatureType, pixel>* Image::computeFeaturesOn(const Rect& pos, std::pair<FeatureType, pixel>* output);
	using namespace Rectfuncs;
	unsigned xm, ym;

	//case FeatureType::vsplit
	xm = xfraction(pos);
	output[0] = std::make_pair( FeatureType::vsplit,
				at({xm, bottom(pos)}) - at({xm, top(pos)}) - at(bottomRight(pos)) + at(topRight(pos)));

	//case FeatureType::hsplit
	ym = yfraction(pos);
	output[1] = std::make_pair( FeatureType::hsplit,
				at(bottomRight(pos)) - at(bottomLeft(pos)) - at({right(pos), ym}) + at({left(pos), ym}));

	//case FeatureType::cantor
	xm = xfraction(pos);
	ym = xfraction(pos, 3, 2);
	output[2] = std::make_pair( FeatureType::cantor,
				at(bottomRight(pos)) - at(topRight(pos)) - at({ym, bottom(pos)}) + at({ym, top(pos)})
			  + at({xm, bottom(pos)}) + at(topLeft(pos)) - at({xm, top(pos)}) - at(bottomLeft(pos)));

	//case FeatureType::checked
	xm = xfraction(pos);
	ym = yfraction(pos);
	output[3] = std::make_pair( FeatureType::checked,
				2*at({xm, ym}) - at({xm, top(pos)}) - 2*at({right(pos),ym})  + at(topRight(pos))
			  + at(bottomRight(pos)) //- at({right(pos), ym})
			  - at({xm, bottom(pos)}));// + at({xm, ym})

	return output;
}

PMImage::PMImage(const pixel* dat, const int& w, const int& h) : Pixelmap(dat, w, h) {}
PMImage::PMImage(const int& w, const int& h) : Pixelmap(w, h) {}
PMImage::PMImage(const char* const filename) : Pixelmap(filename) {}
PMImage::PMImage(const PMImage& other) : Pixelmap(other) {}
PMImage::PMImage(const Pixelmap& other) : Pixelmap(other) {}

