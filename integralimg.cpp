#include "integralimg.h"

//Image::Image() {
//	pixel* dat = data();
//
//	for(int i=1; i< width(); i++) {
//		dat[i] += dat[i-1];
//	}
//	for(int j=1; j< height(); j++) {
//		dat[j*width()] += dat[(j-1)*width()];
//	}
//	pixel s;
//	for(int i=1; i< width(); i++) {
//		s = dat[i];
//		for(int j=1; j< height(); j++) {
//			s += dat[i+j*width()];
//			dat[i+j*width()] = s + dat[i-1+j*width()];
//		}
//	}
//} 

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

std::pair<FeatureType, pixel>* Image::computeFeaturesOn(const Rect& pos, std::pair<FeatureType, pixel>* output) {
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

