#ifndef IMAGE_H
#define IMAGE_H

#include <mpi.h>
#include <CImg.h>
#include "feature.h"
#include <utility>

typedef cimg_library::CImg<pixel> Pixelmap;

class Image {
public:
	virtual pixel* data() const=0;
	virtual uint32_t width() const=0;
	virtual uint32_t height() const=0;

	virtual inline pixel& at(const int& x, const int& y) const {return data()[x+y*width()];}
	virtual inline pixel& operator()(const int& x, const int& y) const {return at(x,y);}

	virtual inline pixel& at(const Point& p) const {return at(p.x,p.y);}
	virtual inline pixel& operator()(const Point& p) const {return at(p);}

	static Image* integral(Image* input, Image* output);
	inline Image* integral(Image* output);

	Features* features(Image* integralbuffer);
protected:
	void computeFeaturesOn(const Rect& pos, const Image* integralofthis, std::pair<FeatureType, pixel>* output);
};

class PMImage : public Image, public Pixelmap {
public:
	PMImage(const pixel* dat, const int& w, const int& h);
	PMImage(const int& w, const int& h);
	PMImage(const char* const filename);
	PMImage(const PMImage& other);
	PMImage(const Pixelmap& other);

	~PMImage(); 

	virtual pixel* data() const { return (pixel*)Pixelmap::data(); };
	virtual uint32_t width() const { return Pixelmap::width(); };
	virtual uint32_t height() const { return Pixelmap::height(); };

	inline PMImage* integral();

	inline Features* features() { return Image::features(init_integralbuffer()); }
protected:
	PMImage* integralbuffer;
	PMImage* init_integralbuffer();
};

#endif
