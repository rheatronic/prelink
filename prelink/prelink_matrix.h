#ifndef __PRELINK_MATRIX_H__
#define __PRELINK_MATRIX_H__

#include "bitflix/bitflix_debug.h"

#include "prelink_state.h"

class Matrix : public State {
public:
	Matrix(double pMatrix[4][4]);
	virtual void write(std::ostrstream &out);
private:
	double matrix[4][4];
};

#endif
