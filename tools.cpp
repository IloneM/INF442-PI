#include "tools.h"
#include <iostream>

namespace Rectfuncs {
	Point bottomLeft(const Rect& r) {
		Point res;
		res.y = bottom(r);
		res.x = left(r);
		return res;
	}

	Point topRight(const Rect& r) {
		Point res;
		res.y = top(r);
		res.x = right(r);
		return res;
	}
}

Grid::Grid(const Rect& _area, const uint32_t _xstep, const uint32_t _ystep, const Point& tl) :
		area(_area), xstep(_xstep), ystep(_ystep) {
	if(tl.x< Rectfuncs::left(_area) or tl.y < Rectfuncs::top(_area))
		realarea.topLeft = _area.topLeft;
	else
		realarea.topLeft = tl;
	realarea.bottomRight.x = tl.x + ((area.bottomRight.x - tl.x) / xstep) * xstep;
	realarea.bottomRight.y = tl.y + ((area.bottomRight.y - tl.y) / ystep) * ystep;

	endpoint = Rectfuncs::bottomLeft(realarea);
	endpoint.y += ystep;
}

Grid::iterator Grid::fromindex(uint32_t ind) {
	if(ind>cardsize())
		return end();
	return iterator(this, {Rectfuncs::left(realarea) + ind % cardwidth()*xstep, Rectfuncs::top(realarea) + ind / cardwidth()*ystep});
}

bool Grid::iterator::operator==(const Grid::iterator& other) {
	return (parent == other.parent) and (it.x == other.it.x) and (it.y == other.it.y);
}

bool Grid::iterator::operator<=(const Grid::iterator& other) {
	return parent == other.parent and (it.y <= other.it.y or (it.y == other.it.y and it.x <= other.it.x));
}

bool Grid::iterator::operator<(const Grid::iterator& other) {
	return parent == other.parent and (it.y < other.it.y or (it.y == other.it.y and it.x < other.it.x));
}

Grid::iterator& Grid::iterator::operator++(int step) {
	it.x += parent->xstep;
	if(it.x > Rectfuncs::right(parent->realarea)) {
		it.x = Rectfuncs::left(parent->realarea);
		it.y += parent->ystep;
	}
	return *this;
}

