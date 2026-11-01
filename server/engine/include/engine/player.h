#pragma once

#include "common/types.hpp"

#include <string>

namespace botf {

// Connected player owned by ServerEngine (TCP session identity).
class Player {
public:
	explicit Player(common::ID id);

	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;

	[[nodiscard]] common::ID getId() const;
	void setId(common::ID id);

	[[nodiscard]] const std::string& getName() const;
	void setName(std::string name);

private:
	common::ID m_id{};
	std::string m_name{};
};

}  // namespace botf
