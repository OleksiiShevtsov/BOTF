#pragma once

#include "common/types.hpp"

namespace botf::app {

class Player {
public:
	Player();
	explicit Player(common::ID id);

	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;

	[[nodiscard]] common::ID id() const;
	void setId(common::ID id);

private:
	common::ID m_id{0};
};

}  // namespace botf::app
