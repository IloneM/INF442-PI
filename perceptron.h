#define PERCEPTRON_H
#define PERCEPTRON_H

#include "feature.h"

typedef double weights_t;
typedef uint8_t featclass_t

struct PerceptronWeights {
	weights_t wa; // aka wi1
	weights_t wb;
}

class Perceptron {
public:
	Perceptron(const weights_t& _eps) : eps(_eps), wa(1), wb(0) {}

	inline featclass_t operator()(const pixel& feature) { return (wa * feature + wb >= 0?1:-1); }

	void train(const pixel& feature, const featclass_t& fc) {
		wa -= eps * (operator(pixel) - fc) * feature;
		wb -= eps * (operator(pixel) - fc);
	}

	PerceptronWeights getWeights(){ return {wa, wb};}


protected:
	weights_t wa; // aka wi1
	weights_t wb; // aka wi2
	weights_t eps;
};

#endif
