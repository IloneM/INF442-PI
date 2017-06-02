#ifndef IMAGE_H
#define IMAGE_H

#include <mpi.h>
#include <CImg.h>
#include "feature.h"
#include <utility>
#include <vector>

typedef cimg_library::CImg<pixel> Pixelmap;

class Image {
public:
//	Image() { integral(this, this);  }
//	Image();
	
	virtual pixel* data()=0;
	virtual pixel* data() const=0;
	virtual uint32_t width() const=0;
	virtual uint32_t height() const=0;
	virtual void load(const char* path)=0;

	virtual inline pixel& at(const int& x, const int& y) const {return data()[x+y*width()];}
	virtual inline pixel& operator()(const int& x, const int& y) const {return at(x,y);}

	virtual inline pixel& at(const Point& p) const {return at(p.x,p.y);}
	virtual inline pixel& operator()(const Point& p) const {return at(p);}

	static Image* integral(const Image* input, Image* output);

	std::pair<FeatureType, pixel>* computeFeaturesOn(const Rect& pos, std::pair<FeatureType, pixel>* output);
protected:
	bool initialized;
};

class PMImage : public Image, public Pixelmap {
public:
	PMImage(const pixel* dat, const int& w, const int& h);
	PMImage(const int& w, const int& h);
	PMImage(const char* const filename);
//	inline PMImage(const std::string& filename) : PMImage(filename.c_str()) {};
	PMImage(const PMImage& other);
	PMImage(const Pixelmap& other);

	virtual void load(const char* path) { Pixelmap::load(path); }
	virtual pixel* data() { return (pixel*)Pixelmap::data(); }
	virtual pixel* data() const { return (pixel*)Pixelmap::data(); }
	virtual uint32_t width() const { return Pixelmap::width(); }
	virtual uint32_t height() const { return Pixelmap::height(); }

//	inline Features* features() { return Image::features(init_integralbuffer()); }
};

#endif

