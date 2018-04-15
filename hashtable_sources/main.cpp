#define BRICK_BENCHMARK_REG
#define BRICK_BENCHMARK_MAIN

/* ./bench category:stack | gnuplot > soubor.pdf */

#include <brick-benchmark>
#include <cmath>
#include <cstdlib>
#include <random>
#include <set>
#include <unordered_set>
#include <type_traits>

#include "chained_hash_table.hpp"
#include "linear_probing_hash_table.hpp"

using namespace brick;
using T = int;
using set = std::set< T >;
using uset = std::unordered_set< T >;
using cht = chained_hash_table< T >;
using pht = linear_probing_hash_table< T >;

struct hw2 : benchmark::Group {
	hw2() {
		x.type = benchmark::Axis::Quantitative;
        x.name = "items";
        x.min = 1000;
        x.max = 10000;
		x.log = false;
        x.step = 1000;

		y.type = benchmark::Axis::Qualitative;
		y.name = "implementation";
		y.min = 1;
        y.max = 4;
        y._render = [](int i) {
            switch (i) {
            case 1: return "unordered_set";
            case 2: return "hash_table(chaining)";
			case 3: return "hash_table(linear probing)";
			case 4: return "set";
            }
        };
	}

	template < typename C >
	void _insert() const {
		C con;
		if constexpr (std::is_same< C, uset >::value || std::is_same< C, pht >::value) {
			con.max_load_factor(2.0f/3.0f);
		}
		for (int i = 0; i < p; ++i)
				con.insert(_data[i]);
	}

	std::vector< T > _data;
};

struct insert : hw2 {
	BENCHMARK(compare) {
		switch (q) {
		case 1: _insert< uset >(); break;
		case 2: _insert< cht >(); break;
		case 3: _insert< pht >(); break;
		case 4: _insert< set >(); break;
       	}
	}

	BENCHMARK(time_per_insert) {
		x.normalize = benchmark::Axis::Div;
		switch (q) {
		case 1: _insert< uset >(); break;
		case 2: _insert< cht >(); break;
		case 3: _insert< pht >(); break;
		case 4: _insert< set >(); break;
       	}
	}

	void setup(int _pt, int _q) override {
		p = _pt	; q = _q;

		_data.resize(p);
		
		std::random_device r;
		std::default_random_engine e(r());
		std::uniform_int_distribution< T > uid(0, 2 * x.max);

		
		for (auto& d : _data) {
			d = uid(e);
		}
	}
};

struct erase : hw2 {
	
	BENCHMARK(compare) {
		std::random_device r;
		std::mt19937 mt(r());
		switch (q) {
		case 1: _u.erase(mt() % p); break;
		case 2: _p.erase(mt() % p); break;
		case 3: _c.erase(mt() % p); break;
		case 4: _s.erase(mt() % p); break;
		}
	}

	void setup(int _pt, int _q) override {
		p = _pt	; q = _q;
		
		std::random_device r;
		std::default_random_engine e(r());
		std::uniform_int_distribution< T > uid(0, x.max);
		
		for (int i = 0; i < p; ++i) {
			auto x = uid(e);
			_u.insert(x);
			_p.insert(x);
			_c.insert(x);
			_s.insert(x);
		}		
	}

	uset _u;
	pht _p;
	cht _c;
	set _s;
};

struct find : hw2 {
	BENCHMARK(compare) {
		std::random_device r;
		std::mt19937 s(r());
		switch (q) {
		case 1: _u.find(s()); break;
		case 2: _p.find(s()); break;
		case 3: _c.find(s()); break;
		case 4: _s.find(s()); break;
		}
	}

	void setup(int _pt, int _q) override {
		p = _pt	; q = _q;
		
		std::random_device r;
		std::default_random_engine e(r());
		std::uniform_int_distribution< T > uid(0, x.max);

		_data.resize(p);
		_u = uset();
		_p = pht();
		_c = cht();
		_s = set();
		
		for (int i = 0; i < p; ++i) {
			auto x = uid(e);
			_u.insert(x);
			_p.insert(x);
			_c.insert(x);
			_s.insert(x);
		}		
	}
	
	uset _u;
	pht _p;
	cht _c;
	set _s;
};

#include <queue>
#include <list>

struct queue : benchmark::Group {
	queue() {
		x.type = benchmark::Axis::Quantitative;
        x.name = "items";
        x.min = 4;
        x.max = 2000;
		x.log = true;
        x.step = 4;

		y.type = benchmark::Axis::Qualitative;
		y.name = "implementation";
		y.min = 1;
        y.max = 2;
        y._render = [](int i) {
            switch (i) {
            case 1: return "deque";
            case 2: return "list";
            }
        };
	}

	BENCHMARK(push) {
		switch (q) {
		case 1: pb< std::deque >(); break;
		case 2: pb< std::list >(); break;
		}	
	}

	template < template <typename, typename > typename C >
	void pb() const {
		std::queue< T, C< T, std::allocator< T > > > q;
		for (int i = 0; i < p; ++i) {
			if (i % 3 == 0 && !q.empty())
				q.pop();
			else
				q.push(i);
		}
	}
};

#include "graph.hpp"

struct bfs : benchmark::Group {
	bfs() {
		x.type = benchmark::Axis::Quantitative;
        x.name = "graph size";
		x.unit = "amount of nodes";
        x.min = 1024;
        x.max = 4096;
		x.log = true;
        x.step = 4;
	}

	void setup(int _p, int _q) override {
		p = _p; q = _q;
		g = generate(p);
	}

	BENCHMARK(deque) {
		bfs(g);
	}

	graph g;

};
