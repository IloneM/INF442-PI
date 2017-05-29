#include "feature.h"

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

//NOTE: cannot use here operator== for Rect and Point as they must be simple for MPI to use them as buffer
bool FeatureKey::operator==(const FeatureKey& other) const {
	return  ft == other.ft
			and pos.topLeft.x == other.pos.topLeft.x and pos.topLeft.y == other.pos.topLeft.y
			and pos.bottomRight.x == other.pos.bottomRight.x and pos.bottomRight.y == other.pos.bottomRight.y;
}

namespace std {
	size_t hash<Point>::operator()(const Point& p) const
    {
	  std::size_t seed = 0;

	  hash_combine<uint32_t>(seed, p.x);
	  hash_combine<uint32_t>(seed, p.y);

	  return seed;
    }

    size_t hash<Rect>::operator()(const Rect& r) const
    {
	  std::size_t seed = 0;

	  hash_combine<Point>(seed, r.topLeft);
	  hash_combine<Point>(seed, r.bottomRight);

	  return seed;
    }

    size_t hash<FeatureKey>::operator()(const FeatureKey& k) const
    {
	  std::size_t seed = 0;

	  hash_combine<Rect>(seed, k.pos);
	  hash_combine<FeatureType>(seed, k.ft);

	  return seed;
    }
}

