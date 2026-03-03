#pragma once

#include "MenuList.h"
#include "Game.h"
#include "Player.h"
#include <functional>

class MenuLoad : public MenuList
{
public:
	virtual void draw_screen() override;
	virtual void handle_input (int key) override;
	void refresh();
protected:
	struct FileMetadata
	{
		bool valid = false;
		Game::VersionNumber version {};
		Player::Data player_data {};
		int turn_number = 0;
	};

	std::vector<FileMetadata> metadata_list;

	static FileMetadata get_metadata(std::string const& stem);
};
