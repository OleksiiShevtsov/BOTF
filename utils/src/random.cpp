#include "utils/random.hpp"

#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>

namespace {

std::mt19937& rng() {
	thread_local std::mt19937 engine{std::random_device{}()};
	return engine;
}

}  // namespace

void botf::utils::seedRandom(const std::uint32_t seed) {
	rng().seed(seed);
}

std::size_t botf::utils::randomIndex(const std::size_t count) {
	if (count == 0) {
		throw std::runtime_error("utils::randomIndex: count == 0");
	}
	std::uniform_int_distribution<std::size_t> dist{0, count - 1};
	return dist(rng());
}

std::vector<std::size_t> botf::utils::randomIndices(const std::size_t count) {
	std::vector<std::size_t> indices(count);
	std::iota(indices.begin(), indices.end(), std::size_t{0});
	for (std::size_t i = count; i > 1; --i) {
		const std::size_t j = randomIndex(i);
		std::swap(indices[i - 1], indices[j]);
	}
	return indices;
}
