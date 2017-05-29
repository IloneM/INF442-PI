#include <CImg.h>

typedef signed char pixel;

//typedef pixel* Image;

class Image {
public:
	Image(int width, int height) {}
	Image(char* path) {}

	virtual const int width()=0;
	virtual const int height()=0;

	virtual pixel* data()=0;

	virtual pixel& at(int x, int y)=0; 
	virtual inline pixel& operator() (int x, int y) {return at(x,y);}

	inline Image& integral() {return integral(*this);}

	Image& integral(Image& other) {
		if(!width() or !height())
			return NULL;
		Image* result = new Image(width(), height());

		pixel* res_dat = result->data();
		pixel* raw_dat = other.data();

		res_dat[0] = raw_dat[0];
		for(int i=1; i< width(); i++) {
			res_dat[i] = res_dat[i-1] + raw_dat[i];
		}
		for(int j=1; j< height(); j++) {
			res_dat[j*width()] = res_dat[(j-1)*width()] + raw_dat[j*width()];
		}
		pixel s;
		for(int i=1; i< width(); i++) {
			s = res_dat[i];
			for(int j=1; j< height(); j++) {
				s += raw_dat[i+j*width()];
				res_dat[i+j*width()] = s + res_dat[i-1+j*width()];
			}
		}

		return *result;
	}
};

using namespace cimg_library;

typedef CImg<pixel> Pixelmap;

class CImgImage : public Image {
public:
	virtual Image(int width, int height) : core(width, height) {}

	virtual Image(char* path) : core(path) {}

	virtual inline const int width() {return core.width();}
	virtual inline const int height() {return core.height();}

	virtual pixel* data() {return core.data();}

	virtual inline const pixel& at(int x, int y) {return core(x,y);} 
	virtual inline pixel& at(int x, int y) {return core(x,y);} 
private:
	Pixelmap core;
}

//Image integral_image(Image raw, int width, int height) {
//	if(!width or !height)
//		return NULL;
//	pixel* res = new pixel[width*height];
//
//	res[0] = raw[0];
//	for(int i=1; i< width; i++) {
//		res[i] = res[i-1] + raw[i];
//	}
//	for(int j=1; j< height; j++) {
//		res[j*width] = res[(j-1)*width] + raw[j*width];
//	}
//	pixel s;
//	for(int i=1; i< width; i++) {
//		s = res[i];
//		for(int j=1; j< height; j++) {
//			s += raw[i+j*width];
//			res[i+j*width] = s + res[i-1+j*width];
//		}
//	}
//
//	return res;
//}

