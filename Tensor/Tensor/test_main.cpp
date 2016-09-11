#include <iostream>
#include <vector>
#include "tensor.h"
#include "ConcreteTensor.h"
#include "FunctionTensor.h"
#include "ReferenceTensor.h"
#include "tensor_operators.h"
#include "util.h"

double func(int i, double t) {
	//std::shared_ptr<tensor<double>>(
	//	new tensor<double>(std::vector<double>{1, 2, 3}, tu::s("i", 3)));
	return  (1+i)*t;
}

int main() {

	std::vector<double> v{1,2,3};
	std::shared_ptr<tensor<double> > a(new ConcreteTensor<double>(v, tu::s("i", 3), tu::s("i", 1)));
	std::shared_ptr<tensor<double> > b(new ConcreteTensor<double>(v, tu::s("j", 3)));
	std::shared_ptr<tensor<double> > c(new ConcreteTensor<double>(v, tu::s("k", 3)));
	std::shared_ptr<tensor<double> > f(new ConcreteTensor<double>(v, tu::s("l", 3)));
	std::shared_ptr<tensor<double> > d(new ReferenceTensor<double>(1, tu::s("j", 3)));
	//std::shared_ptr<tensor<double> > d(new ReferenceTensor<double>(1, tu::s("l", 5, "k", 5)));
	std::shared_ptr<tensor<double> > e(new FunctionTensor<double>(func, tu::s("j", 3)));

	a->view();

	(e*(a+b+c+f))->view();

//	(*((*d*2.))*(*e * 3.))->view();

	// add
	//a + b//->view();
	////(*(*a + b) + c)->view();

	//// multiply
	//(*(*a * b) * c)->view();
}
