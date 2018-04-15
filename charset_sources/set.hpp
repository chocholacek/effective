#pragma once

#include <bitset>
#include <vector>


class set1 {
public:
	set1() = default;
	void insert(uint16_t val) noexcept {
		_data[val] = true;
	}

	void erase(uint16_t val) noexcept {
		_data[val] = false;
	}

	set1 operator&(const set1& o) noexcept {
		return { _data & o._data };
	}

	set1 operator|(const set1& o) noexcept {
		return { _data | o._data };
	}

private:
	std::bitset< UINT16_MAX > _data;
	set1(decltype(_data)&& bs) : _data(bs) {}
};

class set2 {
	struct node {
		using bs = std::bitset< 16 >;

		node() = default;
		node(unsigned short degree)
				: degree(degree) {
			if (degree > 0) {
				succs = std::vector< node >(16, degree - 1);
			}
		}

		operator bool() {
			return accepting;
		}


		std::vector< node > succs;
		bool accepting = false;
		unsigned short degree;
	};

	struct fields {
		fields(uint16_t val)
				: a(val),
				  b(val >> 4),
				  c(val >> 8),
				  d(val >> 12) {}
		uint8_t operator[](unsigned short x) noexcept {
			switch (x) {
			case 0: return a;
			case 1: return b;
			case 2: return c;
			case 3: return d;
			}
		}

	private:
		uint16_t a : 4, b : 4, c : 4, d : 4;
	};
public:
	set2() : _root(4) {}

	void insert(fields val) {
		node* n = &_root;
		for (unsigned i = 0; i < 4; ++i) {
			n = &n->succs[val[i]];
		}
		n->accepting = true;
	}

	void erase(fields val) {
		node* n = &_root;
		for (unsigned i = 0; i < 4; ++i) {
			n = &n->succs[val[i]];
		}
		n->accepting = false;
	}

	set2 operator&(const set2& o) {
		node n(4);
		traverse(&_root, &o._root, &n, [](bool a, bool b){ return a && b; });
		return set2(std::move(n));
	}

	set2 operator|(const set2& o) {
		node n(4);
		traverse(&_root, &o._root, &n, [](bool a, bool b){ return a || b; });
		return set2(std::move(n));
	}
private:
	node _root;
	set2(node&& n) : _root(std::move(n)) {}

	template < typename F >
	void traverse(const node* n1, const node* n2, node* target, F f) {
		if (!n1->degree) {
			target->accepting = f(n1->accepting, n2->accepting);
		} else {
			for (unsigned i = 0; i < 16; ++i)
				traverse(&n1->succs[i], &n2->succs[i], &target->succs[i], f);
		}
	}
};

#include <random>

template < typename T >
T generate(std::size_t size) {
	std::random_device r;
	std::default_random_engine e(r());
	std::uniform_int_distribution< uint16_t > uid(0, UINT16_MAX);
	T t;
	for (auto s = size; s > 0; --s)
		t.insert(uid(e));
	return t;
}
