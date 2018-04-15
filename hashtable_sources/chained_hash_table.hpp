#pragma once

#include <functional>
#include <list>
#include <vector>


template < typename Key, typename Hash = std::hash< Key >, typename KeyEqual = std::equal_to< Key > >
class chained_hash_table {
	using list = std::list < Key >;
	using vector = std::vector< list >;
	
	template < typename Base1, typename Base2, typename T >
	struct _iterator {
	protected:
		Base1 _s = nullptr, _e = nullptr;
		Base2 _c;

	public:
		_iterator() = default;
		_iterator(Base1 s, Base1 e)
				: _s(s), _e(e) {
			while (_s != _e && _s->empty())
				++_s;
			if (_s != _e)
				_c = _s->begin();
		}

		_iterator(Base1 s, Base1 e, Base2 c)
				: _s(s), _e(e), _c(c) {}

		explicit operator Base1() {
			return _s;
		}

		explicit operator Base2() {
			return _c;
		}

		T& operator*() const noexcept {
			return *_c;
		}

		T* operator->() const noexcept {
			return std::addressof(*_c);
		}

		_iterator& operator++() noexcept {
			if (++_c == _s->end()) {
				do {
					++_s;
				} while (_s != _e && _s->empty());
			}
			return *this;
		}

		_iterator operator++(int) {
			auto cpy = *this;
			++(*this);
			return cpy;
		}

		bool operator==(const _iterator& i) const noexcept {
			return _s == i._s && _e == i._e && _c == i._c;
		}

		bool operator!=(const _iterator& i) const noexcept {
			return !(*this == i);
		}
	};
public:
	using iterator = _iterator< typename vector::iterator, typename list::iterator, const Key >;
	using const_iterator = _iterator< typename vector::const_iterator, typename list::const_iterator, const Key >;

	chained_hash_table()
			: _ml_factor(10),
			  _data(1) {}

	void rehash(std::size_t count) {
		auto tmp = std::move(_data);
		_data.resize(count);
		auto olen = _entries;
		_entries = 0;
		for (auto& l : tmp) {
			if (olen == _entries)
				break;
			for (auto& k : l) {
				_data[Hash()(k) % size()].push_back(std::move(k));
				++_entries;
			}
		}
	}

	const_iterator find(const Key& k) const {
		auto lit = _data.begin() + Hash()(k) % size();
		for (auto it = lit->begin(); it != lit->end(); ++it) {
			if (KeyEqual()(*it, k))
				return const_iterator(lit, _data.end(), it);
		}
		return end();
	}

	iterator find(const Key& k) {
		auto lit = _data.begin() + Hash()(k) % size();
		for (auto it = lit->begin(); it != lit->end(); ++it) {
			if (KeyEqual()(*it, k))
				return iterator(lit, _data.end(), it);
		}
		return end();
	}

	bool erase(const Key& k) noexcept {
		if (auto it = find(k); it != end()) {
			static_cast< typename vector::iterator >(it)->erase(static_cast< typename list::iterator >(it));
			return true;
		}
		return false;
	}

	bool insert(const Key& k) {
		return _insert(k);
	}

	bool insert(Key&& k) {
		return _insert(std::move(k));
	}

	std::size_t size() const noexcept {
		return _data.size();
	}

	float load_factor() const noexcept {
		return _entries / size();
	}

	void max_load_factor(float ml) noexcept {
		_ml_factor = ml;
	}

	iterator begin() {
		return iterator(_data.begin(), _data.end());
	}

	const_iterator begin() const {
		return const_iterator(_data.begin(), _data.end());
	}

	iterator end() {
		return iterator(_data.begin(), _data.end());
	}

	const_iterator end() const {
		return const_iterator(_data.begin(), _data.end());
	}

private:
	template < typename _K >
	bool _insert(_K&& k) {
		std::size_t pos = Hash()(k) % size();
		for (const auto& b : _data[pos]) {
			if (KeyEqual()(k, b))
				return false;
		}
		_data[pos].push_back(std::forward< _K >(k));
		++_entries;
		if (load_factor() > _ml_factor)
			rehash(2 * size());
		return true;
	}

	float _ml_factor;
	vector _data;
	std::size_t _entries = 0;
};

