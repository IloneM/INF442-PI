#ifndef FEATURE_H
#define FEATURE_H

#include <unordered_map>
#include <functional>

//same as in boost
template <class T>
inline void hash_combine(std::size_t& seed, const T& v);

#include "tools.h"

#define NB_DIFF_FEATURES 4

//typedef signed char pixel;
typedef uint32_t pixel;

enum FeatureType {vsplit, hsplit, cantor, checked};

struct Feature {
	Rect pos;
	FeatureType ft;
	pixel value;
};

struct FeatureKey {
	Rect pos;
	FeatureType ft;

	bool operator==(const FeatureKey& other) const;
};

typedef std::unordered_map<FeatureKey, pixel> Features;

namespace std {
  template <>
  struct hash<Point>
  {
    size_t operator()(const Point& p) const;
  };

  template <>
  struct hash<Rect>
  {
    size_t operator()(const Rect& r) const;
  };

  template <>
  struct hash<FeatureKey>
  {
    size_t operator()(const FeatureKey& k) const;
  };
}

inline FeatureKey getKey(Feature& f) {return {f.pos, f.ft};}

inline const pixel& getValue(const Feature& f) {return f.value;}
inline const Rect& getPos(const Feature& f) {return f.pos;}
inline const FeatureType& getFt(const Feature& f) {return f.ft;}

inline pixel& getValue(Feature& f) {return f.value;}
inline Rect& getPos(Feature& f) {return f.pos;}
inline FeatureType& getFt(Feature& f) {return f.ft;}

#endif
