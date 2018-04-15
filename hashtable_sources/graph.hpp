#pragma once

#include <vector>
#include <array>
#include <random>
#include <deque>

using matrix = std::vector< std::vector< bool > >;

template < typename T >
T rand(T min, T max) {
	std::random_device r;
	std::default_random_engine e(r());
	std::uniform_int_distribution< int > d(min, max);
	return d(e);
}

matrix generate(std::size_t n) {
	std::random_device r;
	std::default_random_engine e(r());
	std::uniform_int_distribution< int > d(0, 1);
	matrix m;
	m.resize(n);
	for (auto& v : m)
		v.resize(n);
	for (std::size_t b = 0; b < m.size(); ++b) {
		for (std::size_t a = 0; a < m[b].size(); ++a)
			m[a][b] = static_cast< bool >(rand< std::size_t >(0, 1));
	}
	return m;
}

class graph {
	matrix nodes;

public:
	graph() = default;
	graph& operator=(matrix&& m) noexcept {
		nodes = std::move(m);
	}

	void generate(std::size_t n) {
		nodes.resize(n);
		for (auto& v : nodes)
			v.resize(n);

		for (std::size_t b = 0; b < nodes.size(); ++b) {
			for (std::size_t a = 0; a < nodes[b].size(); ++a)
				nodes[a][b] = static_cast< bool >(rand< std::size_t >(0, 1));
		}
	}

	auto size() const noexcept {
		return nodes.size();
	}

	bool operator()(std::size_t x, std::size_t y) const noexcept {
		return nodes[y][x];
	}

};

void bfs(const graph& g) {
	std::vector< bool > visited(g.size(), false);
	std::deque< std::size_t > q;

	auto s = rand< std::size_t >(0, g.size() - 1);
	visited[s] = true;

	q.push_back(s);
	while (!q.empty()) {
		s = q.front();
		q.pop_front();
		for (std::size_t i = 0; i < g.size(); ++i) {
			if (g(s, i) && !visited[i]) {
				visited[i] = true;
				q.push_back(i);
			}
		}
	}
}


