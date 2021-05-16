#include "prelink_matrix.h"

Matrix::Matrix(double pMatrix[4][4])
{
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			double v = pMatrix[i][j];
			this->matrix[i][j] = v;
		}
}


void Matrix::write(std::ostrstream &out)
{
	out << "<matrix>";
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			out << this->matrix[j][i] << " ";
	out << "</matrix>";
}
