#pragma once

#include "common/dto.hpp"
#include "envelope.pb.h"
#include "input.pb.h"
#include "world_state.pb.h"

namespace botf::protocol {
namespace {

[[nodiscard]] inline botf::LifeStatus toProto(common::LifeStatus status) {
	switch (status) {
	case common::LifeStatus::Dead:
		return botf::LIFE_STATUS_DEAD;
	case common::LifeStatus::Waiting:
		return botf::LIFE_STATUS_WAITING;
	case common::LifeStatus::Alive:
	default:
		return botf::LIFE_STATUS_ALIVE;
	}
}

[[nodiscard]] inline common::LifeStatus fromProto(botf::LifeStatus status) {
	switch (status) {
	case botf::LIFE_STATUS_DEAD:
		return common::LifeStatus::Dead;
	case botf::LIFE_STATUS_WAITING:
		return common::LifeStatus::Waiting;
	case botf::LIFE_STATUS_ALIVE:
	default:
		return common::LifeStatus::Alive;
	}
}

}  // namespace

[[nodiscard]] inline botf::Hello toProto(const common::HelloMessage& hello) {
	botf::Hello message;
	message.set_player_name(hello.playerName);
	message.set_object_id(hello.objectId);
	return message;
}

[[nodiscard]] inline common::HelloMessage fromProto(const botf::Hello& hello) {
	return common::HelloMessage{hello.player_name(), hello.object_id()};
}

[[nodiscard]] inline botf::ClientInput toProto(const common::ClientInput& input) {
	botf::ClientInput message;
	message.set_tick(input.tick);
	message.set_dx(input.dx);
	message.set_dy(input.dy);
	message.set_aim_x(input.aim.x);
	message.set_aim_y(input.aim.y);
	message.set_fire(input.fire);
	return message;
}

[[nodiscard]] inline common::ClientInput fromProto(const botf::ClientInput& input) {
	return common::ClientInput{
		input.dx(),	  input.dy(),	common::Point{input.aim_x(), input.aim_y()},
		input.fire(), input.tick(),
	};
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
		player->set_hp(pose.hp);
		player->set_life_status(toProto(pose.lifeStatus));
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
			player.hp(),
			fromProto(player.life_status()),
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
