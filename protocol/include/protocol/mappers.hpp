#pragma once

#include "aim.pb.h"
#include "common/dto.hpp"
#include "envelope.pb.h"
#include "move.pb.h"
#include "world_state.pb.h"

namespace botf::protocol {

[[nodiscard]] inline botf::Hello toProto(const common::HelloMessage& hello) {
	botf::Hello message;
	message.set_player_name(hello.playerName);
	message.set_object_id(hello.objectId);
	return message;
}

[[nodiscard]] inline common::HelloMessage fromProto(const botf::Hello& hello) {
	return common::HelloMessage{hello.player_name(), hello.object_id()};
}

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

[[nodiscard]] inline botf::Aim toProto(const common::AimCommand& command) {
	botf::Aim aim;
	aim.set_aim_x(command.aim.x);
	aim.set_aim_y(command.aim.y);
	aim.set_tick(command.tick);
	return aim;
}

[[nodiscard]] inline common::AimCommand fromProto(const botf::Aim& aim) {
	return common::AimCommand{common::Point{aim.aim_x(), aim.aim_y()}, aim.tick()};
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
	for (const auto& pose : snapshot.bullets) {
		BulletState* bullet = state.add_bullets();
		bullet->set_id(pose.id);
		bullet->set_owner_id(pose.ownerId);
		auto* position = bullet->mutable_position();
		position->set_x(pose.position.x);
		position->set_y(pose.position.y);
		auto* velocity = bullet->mutable_velocity();
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
	snapshot.bullets.reserve(static_cast<std::size_t>(state.bullets_size()));
	for (const auto& bullet : state.bullets()) {
		snapshot.bullets.push_back(common::BulletPose{
			bullet.id(),
			bullet.owner_id(),
			common::Point{bullet.position().x(), bullet.position().y()},
			common::Point{bullet.velocity().x(), bullet.velocity().y()},
		});
	}
	return snapshot;
}

}  // namespace botf::protocol
