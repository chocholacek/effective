#pragma once

#include <cstdlib>
#include <array>
#include <random>
#include <climits>
#include <algorithm>
#include <memory>
#include <future>

template < typename T> class matrix;

template < typename T >
class matrix_view {
    std::size_t w = 0;
    const T* _data = nullptr;

    friend class matrix< T >;
    T& operator()(std::size_t x, std::size_t y) noexcept {
        return const_cast< T& >(_data[x * w + y]);
    }
public:
    std::size_t width = 0, height = 0;

    matrix_view() = default;

    matrix_view(const matrix< T >& m, std::size_t start, std::size_t end, std::size_t h, std::size_t w)
            : w(m.width()),
              _data(&m.at(start, end)),
              width(w),
              height(h) {}

    const T& operator()(std::size_t x, std::size_t y) const noexcept {
        return _data[x * w + y];
    }

};


template < typename T >
class matrix {
	std::vector< T > _m;
	std::size_t _h = 0;
	std::size_t _w = 0;
public:
	matrix() = default;

	matrix(std::size_t height, std::size_t width)
			: _m(width * height), _h(height), _w(width) {}

    void elems(std::vector< T >&& e) {
        _m = std::move(e);
    }


	T& at(std::size_t x, std::size_t y) {
		if (x >= _h || y >= _w)
			throw std::logic_error("out of bounds");
		return _m[y + x * _w];
	}

	const T& at(std::size_t x, std::size_t y) const {
		return const_cast< matrix* >(this)->at(x, y);
	}

	matrix natural_mul(const matrix& m) const {
		if (width() != m.height())
			throw std::logic_error("dimensions doesn't match");
		std::size_t x = m.width();
		matrix res(_h, x);
		for (std::size_t i = 0; i < _h; ++i) {
			for (std::size_t j = 0; j < x; ++j) {
				T sum = T{};
				for (std::size_t k = 0; k < _w; ++k) {
					sum += _at(i, k) * m._at(k, j);
				}
				res._at(i, j) = sum;
			}
		}
		return res;
	}

	auto horizontal_split() const {
        std::array< matrix_view< T >, 2 > res;
        std::size_t half = std::ceil(static_cast< double >(height()) / 2);
        res[0] = matrix_view< T >(*this, 0, 0, half, width());
        res[1] = matrix_view< T >(*this, half, 0, height() - half, width());
		return res;
	}

    auto vertical_split() const {
        std::array< matrix_view< T >, 2 > res;
        std::size_t half = std::ceil(static_cast< double >(width()) / 2);
        res[0] = matrix_view< T >(*this, 0, 0, height(), half);
        res[1] = matrix_view< T >(*this, 0, half, height(), width() - half);
        return res;
    }

    auto split() {
        std::array< matrix_view< T >, 4 > res;
        std::size_t half_w = std::ceil(static_cast< double >(width()) / 2);
        std::size_t half_h = std::ceil(static_cast< double >(height()) / 2);
        res[0] = matrix_view< T >(*this, 0, 0, half_h, half_w);
        res[1] = matrix_view< T >(*this, 0, half_w, half_h, width() - half_w);
        res[2] = matrix_view< T >(*this, half_h, 0, height() - half_h, half_w);
        res[3] = matrix_view< T >(*this, half_h, half_w, height() - half_w, width() - half_w);
        return res;
    }

	matrix natural_mul_pararell(const matrix& m) const {
		if (width() != m.height())
			throw std::logic_error("dimensions doesn't match");
		std::size_t x = m.width();
		matrix res(_h, x);
        const auto ht = horizontal_split();
        const auto vt = m.vertical_split();
        auto rs = res.split();

        auto f = [&ht, &vt, &rs](unsigned m1, unsigned m2, unsigned r) {
            const auto& rm1 = ht[m1],
                        &rm2 = vt[m2];
            auto& rr = rs[r];
            std::size_t block = 8;
            for (std::size_t i = 0; i < rm1.height; i += block)
                for (std::size_t j = 0; j < rr.width; j += block)
                    for (std::size_t k = 0; k < rm1.width; k += block)
                        for (std::size_t ix = i; ix < std::min(i + block, rm1.height); ++ix)
                            for (std::size_t jx = j; jx < std::min(j + block, rr.width); ++jx) {
                                T sum{};
                                for (std::size_t kx = k; kx < std::min(k + block, rm1.width); ++kx)
                                    sum += rm1(ix, kx) * rm2(kx, jx);
                                rr(ix, jx) = sum;
                            }
        };

        auto f1 = std::async(std::launch::async, f, 0, 0, 0);
        auto f2 = std::async(std::launch::async, f, 1, 0, 2);
        auto f3 = std::async(std::launch::async, f, 0, 1, 1);
        auto f4 = std::async(std::launch::async, f, 1, 1, 3);
        
        return res;
	}


	std::size_t width() const {
		return _w;
	}

	std::size_t height() const {
		return _h;
	}

	matrix cache_mul(const matrix& m) const {
		if (width() != m.height())
			throw std::logic_error("dimensions doesn't match");
		std::size_t x = m.width();
		matrix res(_h, x);
		std::size_t block = 8;
		for (std::size_t i = 0; i < _h; i += block) {
			for (std::size_t j = 0; j < x; j += block) {
				for (std::size_t k = 0; k < _w; k += block) {
					for (auto ix = i; ix < std::min(i + block, _h); ++ix) {
						for (auto jx = j; jx < std::min(j + block, x); ++jx) {
							T sum = T{};
							for (auto kx = k; kx < std::min(k + block, _w); ++kx) {
								sum += _at(ix, kx) * m._at(kx, jx);
							}
							res._at(ix, jx) += sum;
						}
					}
				}
			}
		}
		return res;
	}

	bool operator==(const matrix& m) const {
		if (_w != m.width() || _h != m.height())
			return false;
		return _m == m._m;
	}

private:

	T& _at(std::size_t x, std::size_t y) noexcept {
		return _m[y + x * _w];
	}

	const T& _at(std::size_t x, std::size_t y) const noexcept {
		return const_cast< matrix* >(this)->at(x, y);
	}

};

template < typename T >
auto generate_random_matrix(std::size_t height, std::size_t width) {
	std::random_device r;
	std::mt19937 e(r());
	auto m = std::make_unique< matrix< T > >(height, width);
	for (std::size_t x = 0; x < height; ++x) {
		for (std::size_t y = 0; y < width; ++y) {
			m->at(x, y) = static_cast< short >(e());
		}
	}
	return m;
}


