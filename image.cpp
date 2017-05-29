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

Features* Image::features(Image* integralbuffer) {
	if(!integral(integralbuffer))
		return NULL;

	int rank;
	int size;
	Features* result = NULL;
	Grid tl({{0,0},{width()-MIN_SIZE,height()-MIN_SIZE}}, STEP, STEP);

	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(size > 1) {
		if(rank == 0) {
			size_t vectsize = tl.cardwidth() * (tl.cardwidth()+1)/2
			                  * tl.cardheight() * (tl.cardheight()+1)/2
							  * NB_DIFF_FEATURES;
			result = new Features(vectsize);
			
			Feature receiver;

			for(uint32_t i=0; i< vectsize; i++) {
				MPI_Recv(&receiver, sizeof(Feature), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
				result->emplace(getKey(receiver), getValue(receiver));
			}
		} else {
			uint32_t workerstep = tl.cardsize() / (size-1);

			Grid::iterator tlend;
			if(rank == size-1)
				tlend = tl.end();
			else
				tlend = tl.fromindex(rank*workerstep);
			
			Feature buffer;
			std::pair<FeatureType, pixel> features[NB_DIFF_FEATURES];

			for(Grid::iterator tlit=tl.fromindex((rank-1)*workerstep); tlit < tlend; tlit++) {
				Grid br({{tlit->x + MIN_SIZE, tlit->y + MIN_SIZE},{width(),height()}}, STEP, STEP);

				for(Grid::iterator brit = br.begin(); brit < br.end(); brit++) {
					getPos(buffer) = {*tlit, *brit};
					computeFeaturesOn(getPos(buffer), integralbuffer, features);
					for(int i=0; i< NB_DIFF_FEATURES; i++) {
						getFt(buffer) = features[i].first;
						getValue(buffer) = features[i].second;
						MPI_Send(&buffer, sizeof(Feature), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
					}
				}
			}
		}
	} else {
		size_t vectsize = tl.cardwidth() * (tl.cardwidth()+1)/2
		                  * tl.cardheight() * (tl.cardheight()+1)/2
						  * NB_DIFF_FEATURES;
		result = new Features(vectsize);

		Feature buffer;
		std::pair<FeatureType, pixel> features[NB_DIFF_FEATURES];

		for(Grid::iterator tlit=tl.begin(); tlit < tl.end(); tlit++) {
			Grid br({{tlit->x + MIN_SIZE, tlit->y + MIN_SIZE},{width(),height()}}, STEP, STEP);

			for(Grid::iterator brit = br.begin(); brit < br.end(); brit++) {
				getPos(buffer) = {*tlit, *brit};
				computeFeaturesOn(getPos(buffer), integralbuffer, features);

				for(int i=0; i< NB_DIFF_FEATURES; i++) {
					getFt(buffer) = features[i].first;
					getValue(buffer) = features[i].second;
					result->emplace(getKey(buffer), getValue(buffer));
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

