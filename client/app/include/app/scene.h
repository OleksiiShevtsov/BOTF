#pragma once

#include "app/i_view_object.h"
#include "common/dto.hpp"

#include <cstddef>
#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

namespace botf::app {

// Container for moving IViewObject instances (not the map).
class Scene {
public:
	void clear();
	// Upsert players + bullets by id; remove views absent from the snapshot.
	void upsertFromSnapshot(const common::WorldSnapshot& snapshot);

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] IViewObject* find(common::ObjectID id);
	[[nodiscard]] const IViewObject* find(common::ObjectID id) const;

	void draw(sf::RenderTarget& target) const;

private:
	std::vector<std::unique_ptr<IViewObject>> m_objects;
};

}  // namespace botf::app
