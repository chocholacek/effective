#define BRICK_BENCHMARK_REG
#define BRICK_BENCHMARK_MAIN

/* ./bench category:stack | gnuplot > soubor.pdf */

#include <brick-benchmark>
#include <climits>
#include <string>
#include "matrix.hpp"

using namespace brick;

struct hw5 : benchmark::Group {

	hw5() {
		x.type = benchmark::Axis::Quantitative;
		x.name = "[x][x]";
		x.min = 10;
		x.max = 100;
		x.log = false;
		x.step = 10;


		y.type = benchmark::Axis::Qualitative;
		y.name = "implementation";
		y.min = 2;
		y.max = 3;
		y._render = [](int i) {
		    switch (i) {
		    case 1: return "natural order";
		    case 2: return "cache-efficient order(8)";
			case 3: return "natural order(pararell)";
		    }
		};
	}

	void setup(int _p, int _q) override {
		p = _p; q = _q;
		m1 = generate_random_matrix< double >(p, p);
		m2 = generate_random_matrix< double >(p, p);
	}

	BENCHMARK(multiplication) {
		switch (q) {
		case 1: m1->natural_mul(*m2); break;
		case 2: m1->cache_mul(*m2); break;
		case 3: m1->natural_mul_pararell(*m2); break;
		}
	}

	using mtx_t = matrix< double >;

	std::unique_ptr< mtx_t > m1;
	std::unique_ptr< mtx_t > m2;
};
