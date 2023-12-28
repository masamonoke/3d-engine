#ifndef UTILS_HPP
#define UTILS_HPP

#include <functional>

#define UNUSED(x) (void)(x)

namespace engine {

	template<typename T, typename... Rest>
	void hash_combine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T> {} (v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hash_combine(seed, rest), ...);
	}

}

#endif // UTILS_HPP
