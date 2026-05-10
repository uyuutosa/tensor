#include <iostream>
#include <vector>
#include "Tensor.h"
#include "ConcreteTensor.h"
#include "FunctionTensor.h"
#include "ReferenceTensor.h"
#include "Tensor_operators.h"
#include "util.h"

double func(int i, int j, double t) {
	//std::shared_ptr<Tensor<double>>(
	//	new Tensor<double>(std::vector<double>{1, 2, 3}, tu::s("i", 3)));
	return  (1+i*j)*t;
}

int main() {

	std::vector<double> v{0,1,2};
	std::vector<double> v2{0,1,2,3,4,5,6,7,8,9};
	std::shared_ptr<Tensor<double> > a(new ConcreteTensor<double>(v2, tu::s("i", 10), tu::s("i", 1)));
	std::shared_ptr<Tensor<double> > b(new ConcreteTensor<double>(v2, tu::s("j", 10), tu::s("j", 1)));
	std::shared_ptr<Tensor<double> > c(new ConcreteTensor<double>(v, tu::s("k", 3), tu::s("k", 1)));
	std::shared_ptr<Tensor<double> > d(new ConcreteTensor<double>(v, tu::s("l", 3), tu::s("l", 1)));
	std::shared_ptr<Tensor<double> > g(new ConcreteTensor<double>(v, tu::s("k", 3), tu::s("k", 0)));
	std::shared_ptr<Tensor<double> > t(new ConcreteTensor<double>(v, tu::s("l", 3), tu::s("l", 0)));
	//std::shared_ptr<Tensor<double> > c(new ConcreteTensor<double>(v, tu::s("k", 3)));
	//std::shared_ptr<Tensor<double> > f(new ConcreteTensor<double>(v, tu::s("l", 3)));
	//std::shared_ptr<Tensor<double> > d(new ReferenceTensor<double>(1, tu::s("j", 3)));
	std::shared_ptr<Tensor<double> > o(new ReferenceTensor<double>(1, tu::s("i",3)));
	std::shared_ptr<Tensor<double> > e(new FunctionTensor<double>(func, tu::s("j", 3,"i",3)));

	//(k * (a + b * c/ d) )->view();
	//( (a + b * c/ d) *k)->view();
	auto I = ((a + b));
	auto idx = a + b + c + d;
	auto h = g + t;
	h->view();
	auto ret = (*I)[idx] *h;
	ret->view();
	//;(e * o)->view();
	//(a + b * c)->view();
	//(d - c)->view();
	//(d / c)->view();

	//a->view();

	//(e*(a+b+c+f))->view();

//	(*((*d*2.))*(*e * 3.))->view();

	// add
	//a + b//->view();
	////(*(*a + b) + c)->view();

	//// multiply
	//(*(*a * b) * c)->view();
}
