#include "app/scene.h"

#include "app/bullet_view.h"
#include "app/figure_view.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

void botf::app::Scene::clear() {
	m_objects.clear();
}

void botf::app::Scene::upsertFromSnapshot(const common::WorldSnapshot& snapshot) {
	std::unordered_set<common::ObjectID> seen;
	seen.reserve(snapshot.players.size() + snapshot.bullets.size());

	for (const auto& pose : snapshot.players) {
		seen.insert(pose.id);

		if (IViewObject* existing = find(pose.id)) {
			existing->setPose(pose.position, pose.angle);
			continue;
		}

		auto view = std::make_unique<FigureView>(pose.id);
		view->setPose(pose.position, pose.angle);
		m_objects.push_back(std::move(view));
	}

	for (const auto& pose : snapshot.bullets) {
		seen.insert(pose.id);
		const float angle = std::atan2(pose.velocity.y, pose.velocity.x);

		if (IViewObject* existing = find(pose.id)) {
			existing->setPose(pose.position, angle);
			continue;
		}

		auto view = std::make_unique<BulletView>(pose.id);
		view->setPose(pose.position, angle);
		m_objects.push_back(std::move(view));
	}

	m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
								   [&seen](const std::unique_ptr<IViewObject>& obj) {
									   return !seen.contains(obj->getId());
								   }),
					m_objects.end());
}

std::size_t botf::app::Scene::size() const {
	return m_objects.size();
}

botf::app::IViewObject* botf::app::Scene::find(common::ObjectID id) {
	for (auto& obj : m_objects) {
		if (obj && obj->getId() == id) {
			return obj.get();
		}
	}
	return nullptr;
}

const botf::app::IViewObject* botf::app::Scene::find(common::ObjectID id) const {
	for (const auto& obj : m_objects) {
		if (obj && obj->getId() == id) {
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
