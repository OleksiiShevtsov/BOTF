#pragma once

#include "common/types.hpp"

namespace botf::app {

class Player {
public:
	Player();
	explicit Player(common::ObjectID id);

	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;

	[[nodiscard]] common::ObjectID getId() const;
	void setId(common::ObjectID id);

private:
	common::ObjectID m_id{0};
};

}  // namespace botf::app
