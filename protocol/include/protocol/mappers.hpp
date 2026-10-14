#pragma once

#include "common/dto.hpp"
#include "move.pb.h"
#include "world_state.pb.h"

namespace botf::protocol {

[[nodiscard]] inline botf::Move toProto(const common::MoveCommand& command) {
	botf::Move move;
	move.set_dx(command.dx);
	move.set_dy(command.dy);
	move.set_tick(command.tick);
	return move;
}

[[nodiscard]] inline common::MoveCommand fromProto(const botf::Move& move) {
	return common::MoveCommand{move.dx(), move.dy(), move.tick()};
}

[[nodiscard]] inline botf::WorldState toProto(const common::WorldSnapshot& snapshot) {
	botf::WorldState state;
	state.set_tick(snapshot.tick);
	for (const auto& pose : snapshot.players) {
		PlayerState* player = state.add_players();
		player->set_id(pose.id);
		auto* position = player->mutable_position();
		position->set_x(pose.position.x);
		position->set_y(pose.position.y);
		player->set_aim_angle(pose.angle);
	}
	for (const auto& pose : snapshot.projectiles) {
		ProjectileState* projectile = state.add_projectiles();
		projectile->set_id(pose.id);
		projectile->set_owner_id(pose.ownerId);
		auto* position = projectile->mutable_position();
		position->set_x(pose.position.x);
		position->set_y(pose.position.y);
		auto* velocity = projectile->mutable_velocity();
		velocity->set_x(pose.velocity.x);
		velocity->set_y(pose.velocity.y);
	}
	return state;
}

[[nodiscard]] inline common::WorldSnapshot fromProto(const botf::WorldState& state) {
	common::WorldSnapshot snapshot;
	snapshot.tick = state.tick();
	snapshot.players.reserve(static_cast<std::size_t>(state.players_size()));
	for (const auto& player : state.players()) {
		snapshot.players.push_back(common::PlayerPose{
			player.id(),
			common::Point{player.position().x(), player.position().y()},
			player.aim_angle(),
		});
	}
	snapshot.projectiles.reserve(static_cast<std::size_t>(state.projectiles_size()));
	for (const auto& projectile : state.projectiles()) {
		snapshot.projectiles.push_back(common::ProjectilePose{
			projectile.id(),
			projectile.owner_id(),
			common::Point{projectile.position().x(), projectile.position().y()},
			common::Point{projectile.velocity().x(), projectile.velocity().y()},
		});
	}
	return snapshot;
}

}  // namespace botf::protocol
