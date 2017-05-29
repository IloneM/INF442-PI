#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>

#define MIN_SIZE 8
#define STEP 4
//typedef unsigned int uint32_t;

struct Point {
	uint32_t x;
	uint32_t y;
};

struct Rect {
	Point topLeft;
	Point bottomRight;
};

namespace Rectfuncs {
	inline Point topLeft(const Rect& r) {
		return r.topLeft;
	}

	inline Point bottomRight(const Rect& r) {
		return r.bottomRight;
	}

	Point bottomLeft(const Rect& r);

	Point topRight(const Rect& r);

	inline uint32_t width(const Rect& r) {
		return r.bottomRight.x - r.topLeft.x;
	}

	inline uint32_t height(const Rect& r) {
		return r.bottomRight.y - r.topLeft.y;
	}

	inline uint32_t right(const Rect& r) { return r.bottomRight.x; }
	inline uint32_t left(const Rect& r) { return r.topLeft.x; }
	inline uint32_t bottom(const Rect& r) { return r.bottomRight.y; }
	inline uint32_t top(const Rect& r) { return r.topLeft.y; }

	inline uint32_t xfraction(const Rect& r, uint32_t denominator=2, uint32_t numerator=1) {
		return left(r) + (width(r) * numerator) / denominator;
	}

	inline uint32_t yfraction(const Rect& r, uint32_t denominator=2, uint32_t numerator=1) {
		return top(r) + (height(r) * numerator) / denominator;
	}
}

class Grid {
public:
	Grid(const Rect& _area, const uint32_t _xstep=1, const uint32_t _ystep=1, const Point& tl={0,0});

	inline uint32_t areawidth() const { return Rectfuncs::width(realarea); }
	inline uint32_t areaheight() const { return Rectfuncs::height(realarea); }
	inline uint32_t areasize() const { return areawidth() * areaheight(); }

	inline uint32_t cardwidth() const { return areawidth() / xstep + 1; }
	inline uint32_t cardheight() const { return areaheight() / ystep + 1; }
	inline uint32_t cardsize() const {return cardwidth() * cardheight();}

	class iterator {
		const Grid* parent;

		Point it;
	public:
		iterator(const Grid* _parent, const Point& begin) : parent(_parent), it(begin) {}
		iterator() : parent(NULL) {}
//		iterator(const iterator& other) : parent(other.parent), it(other.it) {}
		bool operator==(const iterator& other);
		inline bool operator!=(const iterator& other) { return ! (*this == other); }
		bool operator<=(const iterator& other);
		bool operator<(const iterator& other);
		iterator& operator++(int step);
		inline iterator& operator++() {return operator++(0);}
		inline Point& operator*() {return it;}
		inline Point* operator->() {return &it;}
	};

	friend class iterator;

	iterator begin() { return iterator(this, realarea.topLeft); }
//	iterator end() { return iterator(this, realarea.bottomRight); }
	iterator end() { return iterator(this, endpoint); }
	iterator fromindex(uint32_t ind);
protected:
	Rect area;//useless?
	Rect realarea;

	Point endpoint;

	uint32_t xstep;
	uint32_t ystep;
};

#endif
