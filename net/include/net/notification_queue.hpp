#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace botf::net {

// Thread-safe FIFO for notifications between the network layer and game logic.
template <typename T>
class NotificationQueue {
public:
	void push(T item) {
		std::lock_guard lock(m_mutex);
		m_queue.push(std::move(item));
	}

	std::optional<T> tryPop() {
		std::lock_guard lock(m_mutex);
		if (m_queue.empty()) {
			return std::nullopt;
		}
		T item = std::move(m_queue.front());
		m_queue.pop();
		return item;
	}

	[[nodiscard]] bool empty() const {
		std::lock_guard lock(m_mutex);
		return m_queue.empty();
	}

	[[nodiscard]] std::size_t size() const {
		std::lock_guard lock(m_mutex);
		return m_queue.size();
	}

private:
	mutable std::mutex m_mutex;
	std::queue<T> m_queue;
};

}  // namespace botf::net
