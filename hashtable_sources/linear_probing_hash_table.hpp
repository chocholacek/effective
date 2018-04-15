#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <stdexcept>

template < typename Key,
		typename Hash = std::hash< Key >,
		typename KeyEqual = std::equal_to< Key > >
class linear_probing_hash_table {
	struct bucket {
		enum class state : uint8_t { empty, occupied, zombie };

		bucket() = default;

		template < typename K >
		bucket(K&& k)
				: _key(k),
				  _state(state::occupied) {}

		bucket(const bucket&) = default;

		bucket(bucket&& b) noexcept
				: _key(std::move(b._key)),
				  _state(b._state) {
			if (b.occupied())
				b._state = state::zombie;
		}

		bucket& operator=(const bucket&) = default;

		bucket& operator=(bucket&& b) noexcept {
			_key = std::move(b._key);
			_state = b._state;
			if (b.occupied())
				b._state = state::zombie;
			return *this;
		}

		template < typename K >
		bucket& operator=(K&& k) {
			_key = std::forward < K >(k);
			_state = state::occupied;
			return *this;
		}

		const Key& value() const noexcept {
			return _key.value();
		}

		bool empty() const noexcept  { return _state == state::empty; }

		bool occupied() const noexcept { return _state == state::occupied ;}

		bool zombie() const noexcept { return _state == state::zombie; }

		void zombify() {
			if (occupied()) {
				_key.reset();
				_state = state::zombie;
			}
		}

	private:
		std::optional< Key > _key;
		state _state = state::empty;
	};

	using vector = std::vector< bucket >;

	template < typename Base, typename T >
	struct _iterator {
	protected:
		Base _cur = nullptr;
		Base _end = nullptr;
	public:
		_iterator() = default;
		_iterator(Base c, Base e)
				: _cur(c), _end(e) {
			if (c != e)
				++(*this);
		}

		T& operator*() const noexcept {
			return _cur->value();
		}

		T* operator->() const noexcept {
			return std::addressof(this->operator*());
		}

		explicit operator Base() const noexcept {
			return _cur;
		}

		_iterator& operator++() noexcept {
			do {
				++_cur;
			} while (_cur != _end && !_cur->occupied());
			return *this;
		}

		_iterator operator++(int) noexcept {
			auto cpy = *this;
			++(*this);
			return cpy;
		}

		bool operator==(const _iterator& i) const noexcept {
			return _cur == i._cur && _end == i._end;
		}

		bool operator!=(const _iterator& i) const noexcept {
			return !(*this == i);
		}
	};
public:
	using iterator = _iterator< typename vector::iterator, const Key >;
	using const_iterator = _iterator< typename vector::const_iterator, const Key >;

	linear_probing_hash_table()
			: _ml_factor(2.0f/3.0f),
			  _data(1) {}

	void rehash(std::size_t count) {
		auto tmp = std::move(_data);
		_data.resize(count);
		auto olen = _entries;
		_entries = 0;
		for (auto& b : tmp) {
			if (_entries == olen)
				break;
			if (b.occupied()) {
				for (auto it = _data.begin() + Hash()(b.value()) % bucket_count(); true; ++it) {
					if (it == _data.end())
						it = _data.begin();
					if (it->empty()) {
						*it = std::move(b);
						++_entries;
						break;
					}
				}
			}
		}
	}

	bool insert(const Key& k) {
		return _insert(k);
	}

	bool insert(Key&& k) {
		return _insert(std::move(k));
	}

	const_iterator find(const Key& k) const {
		auto it = _data.begin() + Hash()(k) % bucket_count();
		for (std::size_t i = 0; i < bucket_count(); ++it, ++i) {
			if (it == _data.end())
				it = _data.begin();
			if (it->empty())
				break;
			if (it->occupied() && KeyEqual()(k, it->value()))
				return end();
		}
		return const_iterator(it, _data.end());
	}

	iterator find(const Key& k) {
		auto it = _data.begin() + Hash()(k) % bucket_count();
		for (std::size_t i = 0; i < bucket_count(); ++it, ++i) {
			if (it == _data.end())
				it = _data.begin();
			if (it->empty())
				break;
			if (it->occupied() && KeyEqual()(k, it->value()))
				return end();
		}
		return iterator(it, _data.end());
	}

	bool erase(const Key& k) {
		if (auto it = find(k); it != this->end()) {
			static_cast< typename vector::iterator >(it)->zombify();
			return true;
		}
		return false;
	}

	std::size_t bucket_count() const noexcept {
		return _data.size();
	}

	float load_factor() const noexcept {
		return _entries / bucket_count();
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
		return iterator(_data.end(), _data.end());
	}

	const_iterator end() const {
		return const_iterator(_data.end(), _data.end());
	}

private:

	template < typename _K >
	bool _insert(_K&& k) {
		auto it = _data.begin() + Hash()(k) % bucket_count();
		for (std::size_t i = 0; i < bucket_count() ; ++it, ++i) {
			if (it == _data.end())
				it = _data.begin();
			if (!it->occupied()) {
				*it = std::forward< _K >(k);
				++_entries;
				while (load_factor() > _ml_factor) {
					rehash(2 * bucket_count());
				}
				return true;
			}
			if (KeyEqual()(k, it->value()))
				return false;
		}
		throw std::logic_error("invalid hash_table");
	}

	float _ml_factor;
	vector _data;
	std::size_t _entries = 0;
};
