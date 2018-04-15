#define BRICK_BENCHMARK_REG
#define BRICK_BENCHMARK_MAIN

#include <brick-benchmark>
#include "set.hpp"

using namespace brick;

struct hw4 : benchmark::Group {
	hw4() : e(r()), uid(0, UINT16_MAX) {
		x.type = benchmark::Axis::Quantitative;
        x.name = "items";
        x.min = 1000;
        x.max = 10000;
		x.log = false;
        x.step = 1000;

		y.type = benchmark::Axis::Qualitative;
		y.name = "implementation";
		y.min = 1;
        y.max = 2;
		y._render = [](int i) {
			switch (i) {
			case 1: return "bitset";
			case 2: return "nibble-trie";
			}
		};
	}

	void setup(int _p, int _q) override {
		p = _p; q = _q;
		for (unsigned i = 0; i < p / 2; ++i) {
			auto x = uid(e);
			s1.insert(x);
			s2.insert(x);
		}
	}

	BENCHMARK(insert) {
		switch (q) {
		case 1: _insert< set1 >(); break;
		case 2: _insert< set2 >(); break;
		}
	}

	BENCHMARK(erase) {
		switch (q) {
		case 1: s1.erase(uid(e)); break;
		case 2: s2.erase(uid(e)); break;
		}
	}

	template < typename S >
	void _insert() {
		S s;
		for (int i = 0; i < p; ++i)
			s.insert(uid(e));
	}

	std::random_device r;
	std::default_random_engine e;
	std::uniform_int_distribution< uint16_t > uid;
	set1 s1;
	set2 s2;
};

#include <experimental/optional>

struct union_intersect : hw4 {

	void setup(int _p, int _q) override {
		p = _p; q = _q;
		b1 = generate< set1 >(p);
		b2 = generate< set1 >(p);
		o1 = generate< set2 >(p);
		o2 = generate< set2 >(p);
	}

	BENCHMARK(Union) {
		switch (q) {
		case 1: b1.value() | b2.value(); break;
		case 2: o1.value() | o2.value(); break;
		}
	}

	BENCHMARK(Intersect) {
		switch (q) {
		case 1: b1.value() & b2.value(); break;
		case 2: o1.value() & o2.value(); break;
		}
	}

	template < typename T >
	using opt = std::experimental::optional< T >;

	opt< set1 > b1;
	opt< set1 > b2;
	opt< set2 > o1;
	opt< set2 > o2;
};

