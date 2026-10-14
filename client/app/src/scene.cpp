#include "app/scene.h"

#include "app/player_view.h"

#include <algorithm>
#include <unordered_set>

void botf::app::Scene::clear() {
	m_objects.clear();
}

void botf::app::Scene::upsertFromSnapshot(const common::WorldSnapshot& snapshot) {
	std::unordered_set<common::ID> seen;
	seen.reserve(snapshot.players.size());

	for (const auto& pose : snapshot.players) {
		seen.insert(pose.id);

		if (IViewObject* existing = find(pose.id)) {
			existing->setPose(pose.position, pose.angle);
			continue;
		}

		auto view = std::make_unique<PlayerView>(pose.id);
		view->setPose(pose.position, pose.angle);
		m_objects.push_back(std::move(view));
	}

	m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
								   [&seen](const std::unique_ptr<IViewObject>& obj) {
									   return !seen.contains(obj->id());
								   }),
					m_objects.end());
}

std::size_t botf::app::Scene::size() const {
	return m_objects.size();
}

botf::app::IViewObject* botf::app::Scene::find(common::ID id) {
	for (auto& obj : m_objects) {
		if (obj && obj->id() == id) {
			return obj.get();
		}
	}
	return nullptr;
}

const botf::app::IViewObject* botf::app::Scene::find(common::ID id) const {
	for (const auto& obj : m_objects) {
		if (obj && obj->id() == id) {
			return obj.get();
		}
	}
	return nullptr;
}

void botf::app::Scene::draw(sf::RenderTarget& target) const {
	for (const auto& obj : m_objects) {
		if (obj) {
			obj->draw(target);
		}
	}
}
