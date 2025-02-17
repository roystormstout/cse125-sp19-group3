#pragma once
#include "../../NuklearInit.h"
#define EVADE_INDEX 0
#define PROJ_INDEX 1
#define OMNI_SKILL_INDEX 2
#define DIR_SKILL_INDEX 3

#define KILLED_TEXT_NUM 4

#define CHEATING_GOLD 45
#define CHEATING_POINTS 3

const char * intToCharArray(int i) {
	string s = std::to_string(i);
	const int n = s.length();
	return s.c_str();
}

// order leaderboard by kills, most first
static void ui_leaderboard(struct nk_context *ctx, struct media *media,
	LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes ) {

	// order kills, usernames, and archetypes by client with most kills first...
	vector<int> kills;							
	vector<string> ordered_usernames;		
	vector<ArcheType> ordered_types;	
	vector<int> curKills = leaderBoard->currentKills;

	// make parallel arrays 'kills' & 'ordered_usernames' having same index for players based on number of kills

	for ( int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total kills for that player
		auto it = std::max_element(curKills.begin(), curKills.end());
		int index = it - curKills.begin();
		int numKills = *it;

		// add next client with most kills username, kills & archetype
		ordered_usernames.push_back(usernames[index]);
		ordered_types.push_back(archetypes[index]);
		kills.push_back(numKills);

		*it = -1;		// reset current max to -1
	}
	nk_style_set_font(ctx, &(media->font_22->handle));
	if (nk_begin(ctx, "Leaderboard", nk_rect(10, 10, 300, 300),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		static const float lbratio[] = { 0.2f, 0.12f, 0.40f,0.28f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, 45, 4, lbratio);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, "Name", NK_TEXT_LEFT);
		nk_label(ctx, "Kills", NK_TEXT_LEFT);
		for (int i = 0; i < GAME_SIZE; i++) {

			const char * player_id;
			string s = std::to_string(i + 1);
			player_id = s.c_str();
			const char * player_point;

			// total kills for player
			string point_s = std::to_string(kills[i]);
			player_point = point_s.c_str();

			nk_layout_row(ctx, NK_DYNAMIC, 36, 4, lbratio);
			nk_label(ctx, player_id, NK_TEXT_LEFT);

			switch (ordered_types[i])	// archetype icon on leaderboard
			{
				case MAGE	 : nk_image(ctx, media->mage);	  break;
				case ASSASSIN: nk_image(ctx, media->assasin); break;
				case WARRIOR : nk_image(ctx, media->warrior); break;
				case KING	 : nk_image(ctx, media->king);	  break;
			}
			
			// username & points
			nk_label(ctx, ordered_usernames[i].c_str(),  NK_TEXT_LEFT);
			nk_label(ctx, player_point,  NK_TEXT_LEFT);
		}

	}
	nk_end(ctx);

	nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
}

static void ui_killphase_header(struct nk_context *ctx, struct media *media, int width, int height, int roundnum, ScenePlayer * player, LeaderBoard * leaderBoard, guiStatus gStatus) {
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	if (nk_begin(ctx, "kill_player_info", nk_rect(10, 310, width * 0.15, 180),
		NK_WINDOW_NO_SCROLLBAR))
	{
		static const float kill_ratio[] = { 0.3f,0.3f, 0.4f };  /* 0.3 + 0.4 + 0.3 = 1 */
		string roundStr = "ROUND: " + std::to_string(roundnum);
		const char * round_char = roundStr.c_str();

		string goldStr = std::to_string(player->gold);
		string vicPtsStr = std::to_string(leaderBoard->currPoints[player->player_id]);
		const char * gold_char = goldStr.c_str();
		const char * vic_char = vicPtsStr.c_str();
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row_dynamic(ctx, 65, 1);
		nk_label(ctx, round_char, NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));

		nk_layout_row(ctx, NK_DYNAMIC, 116,3, kill_ratio);

		nk_spacing(ctx, 1);
		if (nk_group_begin(ctx, "icons", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_image(ctx, media->gold);
			nk_layout_row_static(ctx, 20, 1, 1);
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_image(ctx, media->points);
		}
		nk_group_end(ctx);

		if (nk_group_begin(ctx, "nums", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_label(ctx, gold_char, NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
			nk_layout_row_static(ctx, 20, 1, 1);
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_text(ctx, vic_char, strlen(vic_char), NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
		}
		nk_group_end(ctx);
	}
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}

static void ui_skills(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, vector<nanoseconds> skill_timers) {
	/*
	Q --> Directional Skill (with the exception of King)
	W --> Omni Directional Skill
	E --> Evade (omni)
	R --> Projectile (directional)

	*******    In meta_data.json    ******
	Skills MUST be in the order of: evade (0), projectile (1), omni (2), directional (3)
	*/
	static const char *key_bindings[] = { "Q","W","E","R" };
	static const unsigned int sequential_bindings[] = { DIR_SKILL_INDEX , OMNI_SKILL_INDEX , EVADE_INDEX , PROJ_INDEX };
	static int op = HUMAN;
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	nk_style_set_font(ctx, &(media->font_64->handle));
	if (nk_begin(ctx, "skills", nk_rect(width*0.3,  height*0.84, width*0.4, height*0.16),
		NK_WINDOW_NO_SCROLLBAR))
	{
		static const float ratio[] = { 0.143f,0.143f, 0.143f,0.143f, 0.143f,0.143f, 0.142f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, height *0.16, 7, ratio);
		ArcheType type = player->modelType; 
		for (int i = 0; i < 4; i++) {
			if (nk_group_begin(ctx, key_bindings[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
				nk_layout_row_dynamic(ctx, width *0.05, 1); // nested row
				std::chrono::nanoseconds nanoSecs = skill_timers[sequential_bindings[i]];
				if (nanoSecs > std::chrono::seconds::zero()){
					auto timeExpr = chrono::duration_cast<chrono::seconds>(nanoSecs);
					//to_string
					string result_string = to_string(timeExpr.count());
					const char * cooldown_char_array = result_string.c_str();
					nk_text(ctx, cooldown_char_array, strlen(cooldown_char_array), NK_TEXT_ALIGN_CENTERED);
				}
				else {
					if (type == WARRIOR) {
						if(i < 2 && player->isSilenced)
							nk_image(ctx, media->warrior_silenced[i]);
						else
							nk_image(ctx, media->warrior_skills[i]);
					}
					else if (type == MAGE) {
						if (i < 2 && player->isSilenced)
							nk_image(ctx, media->mage_silenced[i]);
						else
							nk_image(ctx, media->mage_skills[i]);
					}
					else if (type == ASSASSIN) {
						if (i < 2 && player->isSilenced)
							nk_image(ctx, media->assassin_silenced[i]);
						else
							nk_image(ctx, media->assassin_skills[i]);
					}
					else {
						if (i < 2 && player->isSilenced)
							nk_image(ctx, media->king_silenced[i]);
						else
							nk_image(ctx, media->king_skills[i]);
					}
				}
					nk_layout_row_dynamic(ctx, 24, 1);
					nk_text(ctx, key_bindings[i], strlen(key_bindings[i]), NK_TEXT_ALIGN_CENTERED);
				nk_group_end(ctx);
			}
			nk_spacing(ctx, 1);
		}

	}
	nk_end(ctx);
	nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}


static void ui_deadscreen(struct nk_context *ctx, struct media *media, int width, int height, int killTextDeterminant) {
	int deathTextDisplay = killTextDeterminant% KILLED_TEXT_NUM;
	//add a full screen layout that is grey with transparency 0.5
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(20, 20, 20, 140));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(20, 20, 20, 140)));
	if (nk_begin(ctx, "death screen", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR)) {
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row_dynamic(ctx, height*0.4, 1);
		if (deathTextDisplay == 1)
			nk_label(ctx, "You have been smashed!!!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		else if (deathTextDisplay == 2)
			nk_label(ctx, "You have been strangled!!!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		else if (deathTextDisplay == 3)
			nk_label(ctx, "You have been murdered!!!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		else
			nk_label(ctx, "You have been shot through the heart!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
	} 
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}

static void ui_prepare_title(struct nk_context *ctx, struct media *media, int width, int height, char * title, ClientGame * game) {
		nk_layout_row_static(ctx, height*0.042, 10, 1);
		static const float ratio[] = { 0.5f, 0.5f };
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row(ctx, NK_DYNAMIC, 65, 2, ratio);
		nk_text(ctx, title, strlen(title), NK_TEXT_CENTERED);

		if (game->prepareTimer > std::chrono::seconds::zero()) {
			auto timeExpr = chrono::duration_cast<chrono::seconds>(game->prepareTimer);
			int minutes = timeExpr.count() / 60;
			int seconds = timeExpr.count() % 60;
			char buf[100];
			if (seconds < 10) {
				sprintf(buf, "%d:0%d", minutes, seconds);
				if (minutes == 0) {
					ctx->style.text.color = nk_rgba(255, 55, 55, 255);
				}
			}
			else {
				sprintf(buf, "%d:%d", minutes, seconds);
				ctx->style.text.color = nk_rgba(255, 255, 255, 255);
			}
			const char* result = (const char *)buf;
			nk_text(ctx, result, strlen(result), NK_TEXT_CENTERED);
		}
		else {
			game->switchPhase();
		}
		// reset to white
		ctx->style.text.color = nk_rgba(255, 255, 255, 255);
		nk_spacing(ctx, 1);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
}

static void
winner_layout(struct nk_context *ctx, struct media *media, int width, int height,
	LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes, ClientGame* game) {

	// ordered usernames, types, and points for global points leader board
	vector<string> ordered_usernames_global;
	vector<int> points_copy = leaderBoard->currPoints;

	// order global points leaderboard in order of highest points 
	for (int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total points for that player
		auto it = std::max_element(points_copy.begin(), points_copy.end());
		int index = it - points_copy.begin();
		int numPoints = *it;

		ordered_usernames_global.push_back(usernames[index]);					// add username to ordered index

		*it = -1;		// reset current max to -1
	}
	if (game->prepareTimer > std::chrono::seconds::zero()) {
		set_style(ctx, THEME_BLACK);
		if (nk_begin(ctx, "final_victory", nk_rect(0,0, width , height),
			NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(ctx, 0.3*height, 1, 1);
			nk_style_set_font(ctx, &(media->font_64->handle));
			nk_layout_row_dynamic(ctx, 80, 1);
			nk_label(ctx, "WINNER", NK_TEXT_CENTERED);
			nk_layout_row_dynamic(ctx, 80, 1);
			nk_label(ctx, ordered_usernames_global[0].c_str(), NK_TEXT_CENTERED);
			nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
		}
		nk_end(ctx);
		
	}
	else {
		game->switchPhase();
	}
}

static void
summary_layout(struct nk_context *ctx, struct media *media, int width, int height,
	LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes, ClientGame* game) {
	// ordered usernames, types, and points for global points leader board
	vector<string> ordered_usernames_global;
	vector<ArcheType> ordered_types_global;
	vector<int> ordered_points_global;
	vector<int> ordered_kill_global;
	vector<int> points_copy = leaderBoard->currPoints;
	vector<string> placements = { "1st", "2nd", "3rd", "4th" };
	// order global points leaderboard in order of highest points 
	for (int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total points for that player
		auto it = std::max_element(points_copy.begin(), points_copy.end());
		int index = it - points_copy.begin();
		int numPoints = *it;

		ordered_usernames_global.push_back(usernames[index]);					// add username to ordered index
		ordered_types_global.push_back(archetypes[index]);						// add type to ordered index
		ordered_points_global.push_back(leaderBoard->currPoints[index]);		// add points to ordered index
		ordered_kill_global.push_back(leaderBoard->globalKills[index]);			// add gold to ordered index

		*it = -1;		// reset current max to -1
	}
	set_style(ctx, THEME_BLACK);
	const static float finalRatio[] = { 0.18f, 0.18f, 0.18f, 0.18f,0.28f };
	if (nk_begin(ctx, "final_summary", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER)) {
		nk_layout_row(ctx, NK_DYNAMIC, height*0.95, 5, finalRatio);
		for (int i = 0; i < GAME_SIZE; i++) {
			if(nk_group_begin(ctx, ordered_usernames_global[i].c_str(), NK_WINDOW_NO_SCROLLBAR)) {
				nk_style_set_font(ctx, &(media->font_64->handle));
				nk_layout_row_static(ctx, 64, 64, 1);
				nk_layout_row_dynamic(ctx, 64, 1);
				nk_label(ctx, placements[i].c_str(), NK_TEXT_LEFT);
				nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
				nk_layout_row_static(ctx, 64, 64, 1);
				nk_layout_row_static(ctx, 0.1*width, 0.1*width, 1);
				if (ordered_types_global[i] == MAGE)
					nk_image(ctx, media->mage);
				else if (ordered_types_global[i] == WARRIOR)
					nk_image(ctx, media->warrior);
				else if (ordered_types_global[i] == ASSASSIN)
					nk_image(ctx, media->assasin);
				else
					nk_image(ctx, media->king);
				nk_layout_row_dynamic(ctx, 64, 2);
				nk_label(ctx, "ID: ", NK_TEXT_LEFT);
				nk_label(ctx, ordered_usernames_global[i].c_str(), NK_TEXT_LEFT);
				nk_layout_row_static(ctx, 64, 64, 1);

				nk_layout_row_dynamic(ctx, 64, 2);
				nk_label(ctx, "Points: ", NK_TEXT_LEFT);
				nk_label(ctx, to_string(ordered_points_global[i]).c_str(), NK_TEXT_LEFT);
				nk_layout_row_static(ctx, 64, 64, 1);
				nk_layout_row_dynamic(ctx, 64, 2);
				nk_label(ctx, "Total Kills: ", NK_TEXT_LEFT);
				nk_label(ctx, to_string(ordered_kill_global[i]).c_str(), NK_TEXT_LEFT);
			}
			nk_group_end(ctx);
		}

		if (nk_group_begin(ctx, "sudoNerds", NK_WINDOW_NO_SCROLLBAR)) {
			nk_style_set_font(ctx, &(media->font_64->handle));
			nk_layout_row_static(ctx, 64, 64, 1);
			nk_layout_row_dynamic(ctx, 64, 1);
			nk_label(ctx, "Sudo_Nerds", NK_TEXT_LEFT);
			nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
			nk_layout_row_dynamic(ctx, 48, 2);
			nk_label(ctx, "Graphics: ", NK_TEXT_LEFT);
			nk_label(ctx, "Lingfeng Guo", NK_TEXT_LEFT);

			nk_layout_row_dynamic(ctx, 48, 2);			
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_label(ctx, "Jianhan Xu", NK_TEXT_LEFT);

			nk_layout_row_dynamic(ctx, 48, 2);
			nk_label(ctx, "Network: ", NK_TEXT_LEFT);
			nk_label(ctx, "Joshua Chao", NK_TEXT_LEFT);

			nk_layout_row_dynamic(ctx, 48, 2);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_label(ctx, "Yi Hui Chen", NK_TEXT_LEFT);

			nk_layout_row_dynamic(ctx, 48, 2);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_label(ctx, "Xiaoqi Jiang", NK_TEXT_LEFT);


			nk_layout_row_dynamic(ctx, 48, 2);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_label(ctx, "Aldo Malkhassian", NK_TEXT_LEFT);

			nk_layout_row_dynamic(ctx, 48, 2);
			nk_label(ctx, "Music: ", NK_TEXT_LEFT);
			nk_label(ctx, "Clark Phan", NK_TEXT_LEFT);
		}
		nk_group_end(ctx);
	}
	nk_end(ctx);

}


static void ui_kill_info(struct nk_context *ctx, struct media *media, int width, int height, ClientGame * game, guiStatus & gStatus) {
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	if (nk_begin(ctx, "kill_info_updates", nk_rect(width * 0.80, 20, width * 0.20, height*0.3),
		NK_WINDOW_NO_SCROLLBAR))
	{
		std::deque<std::pair<string,string>>::iterator it = gStatus.killUpdates.begin();

		while (it != gStatus.killUpdates.end()) {
			nk_layout_row_dynamic(ctx, 32, 3);
			auto text = (*it++);
			ctx->style.text.color = nk_rgba(55, 255, 55, 255);
			nk_label(ctx, text.first.c_str(), NK_TEXT_LEFT);
			ctx->style.text.color = nk_rgba(235, 235, 235, 255);
			nk_label(ctx, "killed", NK_TEXT_LEFT);
			ctx->style.text.color = nk_rgba(255, 55, 55, 255);
			nk_label(ctx, text.second.c_str(), NK_TEXT_LEFT);
		}
	}
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}


static void ui_kill_timer(struct nk_context *ctx, struct media *media, int width, int height, ClientGame * game, guiStatus gStatus) {
	static const float ratio[] = { 0.3f, 0.4f , 0.3f};
	nk_style_set_font(ctx, &(media->font_64->handle));

	if (game->prepareTimer > std::chrono::seconds::zero()) {
		auto timeExpr = chrono::duration_cast<chrono::seconds>(game->prepareTimer);
			int minutes = timeExpr.count() / 60;
			int seconds = timeExpr.count() % 60;
			char buf[100];
			if (seconds < 10) {
				sprintf(buf, "%d:0%d", minutes, seconds);
				if (minutes == 0) {
					ctx->style.text.color = nk_rgba(255, 55, 55, 255);
				}
			}
			else {
				sprintf(buf, "%d:%d", minutes, seconds);
				ctx->style.text.color = nk_rgba(255, 255, 255, 255);
			}
			const char * result = (const char *)buf;
			nk_layout_row(ctx, NK_DYNAMIC, 65, 3, ratio);
			nk_spacing(ctx, 1);
			nk_text(ctx, result, strlen(result), NK_TEXT_CENTERED);
			nk_spacing(ctx, 1);


			if (minutes == 0 && seconds < 1) {
				ctx->style.text.color = nk_rgba(255, 255, 255, 255);
				nk_style_set_font(ctx, &(media->font_128->handle));
				nk_layout_row_static(ctx, 0.3*height, 1, 1);
				nk_layout_row(ctx, NK_DYNAMIC, 130, 3, ratio);
				nk_spacing(ctx, 1);
				nk_label(ctx, "TIME'S UP!", NK_TEXT_CENTERED);
				nk_spacing(ctx, 1);

			}
			else {
				ctx->style.text.color = nk_rgba(255, 55, 55, 255);
				auto it = gStatus.killStreakUpdates.begin();

				while (it != gStatus.killStreakUpdates.end()) {
					nk_layout_row(ctx, NK_DYNAMIC, 65, 3, ratio);
					auto text = (*it++).first;
					nk_spacing(ctx, 1);
					nk_label(ctx, text.c_str(), NK_TEXT_LEFT);
					nk_spacing(ctx, 1);
				}
			}
	}
	else {
		game->switchPhase();
	}
	// reset to white
	ctx->style.text.color = nk_rgba(255, 255, 255, 255);
	nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
}

static void skill_info_page(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player) {
	struct nk_style *s = &ctx->style;

	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

	switch (player->modelType) {

	case MAGE: // code to be executed if n = 1;
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_image(media->loading_mage));
		//ctx->style.window.fixed_background = nk_style_item_image(media->lobby_background);

		break;
	case ASSASSIN: // code to be executed if n = 2;
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_image(media->loading_assassin));
		//ctx->style.window.fixed_background = nk_style_item_image(media->lobby_background);
		break;
	case KING: // code to be executed if n = 2;
		//ctx->style.window.fixed_background = nk_style_item_image(media->lobby_background);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_image(media->loading_king));
		break;
	case WARRIOR: // code to be executed if n = 2;
		//ctx->style.window.fixed_background = nk_style_item_image(media->lobby_background);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_image(media->loading_warrior));

		break;
	default: // code to be executed if n doesn't match any cases
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_image(media->loading_mage));
		//ctx->style.window.fixed_background = nk_style_item_image(media->lobby_background);

		break;
	}
	if (nk_begin(ctx, "SKILL_INFO", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR
	))
	{ }
	nk_end(ctx);
	nk_style_pop_style_item(ctx);
	
}


static void
kill_layout(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player,
	vector<nanoseconds> skill_timers, LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes,
	int killTextDeterminant, ClientGame* game, guiStatus & gStatus) {
	struct nk_style *s = &ctx->style;
	set_style(ctx, THEME_BLACK);
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	if (nk_begin(ctx, "kill_title", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
		ui_kill_timer(ctx, media, width, height, game, gStatus);
	}
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
	if (!player->isAlive) {
		ui_deadscreen(ctx, media, width, height, killTextDeterminant);
	}
	ui_leaderboard(ctx, media, leaderBoard, usernames, archetypes);

	ui_skills(ctx, media, width, height, player, skill_timers);
	ui_killphase_header(ctx, media, width, height, game->round_number, player, leaderBoard, gStatus);
	ui_kill_info(ctx, media, width, height, game, gStatus);
}




static  void
lobby_layout(struct nk_context *ctx, struct media *media, int width, int height,  ClientGame * game) {
	static bool available = true;
	static bool selected = false;
	static char buf[256] = { 0 };
	set_style(ctx, THEME_RED);

	static ArcheType op = MAGE;
	ctx->style.window.fixed_background = nk_style_item_image(media->lobby_background);

	if (nk_begin(ctx, "Lobby", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR
	))
	{
		ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
		static const char * characterTypeStrings[] = { "HUMAN", "MAGE", "ASSASSIN","WARRIOR","KING" };
		static const float ratio[] = { 0.35f, 0.3f, 0.35f };  /* 0.3 + 0.4 + 0.3 = 1 */
		static const float text_input_ratio[] = { 0.15f, 0.85f };
		static const float choice_ratio[] = { 0.25f, 0.12f, 0.13f, 0.13f, 0.12f,0.25f };
		nk_layout_row_static(ctx, 0.4*height, 15, 1);

		nk_layout_row(ctx, NK_DYNAMIC, 40, 2, ratio);
		nk_text(ctx, "Username: ", 10, NK_TEXT_ALIGN_RIGHT);
		// in window
		nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buf, sizeof(buf) - 1, nk_filter_default);
		nk_layout_row_static(ctx, 0.03*height, 15, 1);
		nk_layout_row(ctx, NK_DYNAMIC, height *0.25, 6, choice_ratio);
		// somewhere out of cycle
		nk_spacing(ctx, 1);
		for (int i = 1; i < 5; i++) {
			if (nk_group_begin(ctx, characterTypeStrings[i], 0)) { // column 1
				nk_layout_row_static(ctx, 128, 128, 1); // nested row

				if (i == WARRIOR)
					nk_image(ctx, media->warrior);
				else if (i == MAGE)
					nk_image(ctx, media->mage);
				else if (i == ASSASSIN)
					nk_image(ctx, media->assasin);
				else
					nk_image(ctx, media->king);
				//nk_layout_row_static(ctx, 0.1*height, 15, 1);
				nk_layout_row_dynamic(ctx, 50, 1);
				if (nk_option_label(ctx, characterTypeStrings[i], op == i)) op = static_cast<ArcheType>(i);

			}
			nk_group_end(ctx);
		}
		//nk_spacing(ctx, 1);

		//horizontal centered
		nk_layout_row(ctx, NK_DYNAMIC, 60, 3, ratio);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Confirm")) {
			buf[6] = '\0';
			//fprintf(stdout, "button pressed, curr selection: %s, curr buf: %s\n", characterTypeStrings[op], buf);
			available = game->sendCharacterSelection(buf, op);
			selected = true;
		}

		nk_spacing(ctx, 1);
	}
	nk_end(ctx);

	if (!available) {
		ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(136, 0, 21, 255));
		if (nk_begin(ctx, "Alert", nk_rect(width*0.33, height*0.3, width*0.33, 200),
			NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_CLOSABLE
		)) {
			ui_widget_centered(ctx, media, 150);
			nk_label(ctx, "It's been taken!", NK_TEXT_ALIGN_CENTERED);
			nk_spacing(ctx, 1);
		}
		else {
			available = true;
		}
		nk_end(ctx);
		selected = false;
	}
	else if (selected) {
		game->switchPhase();
	}
}

// order leaderboard by kills, most first
static void ui_round_results(struct nk_context *ctx, struct media *media,
	LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes, int width, int height, ClientGame * game, guiStatus & guiS) {

	// order kills, usernames, and archetypes by client with most kills first...
	vector<int> kills;
	vector<int> deaths;
	vector<string> ordered_usernames;
	vector<ArcheType> ordered_types;
	vector<int> curKills = leaderBoard->currentKills;
	vector<int> curDeaths = leaderBoard->currentDeaths;
	static const float lbratio[] = { 0.30f, 0.08f, 0.025f, 0.10f, 0.1f, 0.395f };  /* 0.3 + 0.4 + 0.3 = 1 */
	static const float globalLBratio[] = { 0.30f, 0.08f, 0.025f, 0.10f, 0.10f, 0.395f };  /* 0.3 + 0.4 + 0.3 = 1 */
	static const float subTitleratio[] = { 0.30f, 0.70f };  /* 0.3 + 0.4 + 0.3 = 1 */
	static const float btnRatio[] = { 0.97f, 0.03f };
	// make parallel arrays 'kills' & 'ordered_usernames' having same index for players based on number of kills
	for (int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total kills for that player
		auto it = std::max_element(curKills.begin(), curKills.end());
		int index = it - curKills.begin();
		int numKills = *it;

		// add next client with most kills username, kills, points & archetype
		ordered_usernames.push_back(usernames[index]);					// add usernames to ordered index
		ordered_types.push_back(archetypes[index]);						// add types to ordered index
		kills.push_back(numKills);										// add kills to ordered index
		deaths.push_back(curDeaths[index]);								// add deaths to ordered index

		*it = -1;		// reset current max to -1
	}


	// ordered usernames, types, and points for global points leader board
	vector<string> ordered_usernames_global;
	vector<ArcheType> ordered_types_global;
	vector<int> ordered_points_global;
	vector<int> ordered_gold_global;
	vector<int> points_copy = leaderBoard->currPoints;

	// order global points leaderboard in order of highest points 
	for (int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total points for that player
		auto it = std::max_element(points_copy.begin(), points_copy.end());
		int index = it - points_copy.begin();
		int numPoints = *it;

		ordered_usernames_global.push_back(usernames[index]);					// add username to ordered index
		ordered_types_global.push_back(archetypes[index]);						// add type to ordered index
		ordered_points_global.push_back(leaderBoard->currPoints[index]);		// add points to ordered index
		ordered_gold_global.push_back(leaderBoard->currGold[index]);			// add gold to ordered index

		*it = -1;		// reset current max to -1
	}

	if (nk_begin(ctx, "round result", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND))
	{
		ui_prepare_title(ctx, media, width, height, "Summary", game);
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row(ctx, NK_DYNAMIC, height*0.12, 2, subTitleratio);
		nk_spacing(ctx, 1);
		nk_label(ctx, "Round Summary", NK_TEXT_LEFT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
		nk_layout_row(ctx, NK_DYNAMIC, width*0.02, 6, lbratio);
		nk_spacing(ctx, 1);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, "Name", NK_TEXT_LEFT);
		nk_label(ctx, "Kills", NK_TEXT_LEFT);
		nk_label(ctx, "Deaths", NK_TEXT_LEFT);
		for (int i = 0; i < GAME_SIZE; i++) {

			const char * player_id;
			string s = std::to_string(i + 1);
			player_id = s.c_str();
			const char * player_point;

			// total kills for player
			string point_s = std::to_string(kills[i]);
			player_point = point_s.c_str();

			nk_layout_row(ctx, NK_DYNAMIC, width*0.025, 6, lbratio);
			nk_spacing(ctx, 1);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);
			switch (ordered_types[i])	// archetype icon on leaderboard
			{
			case MAGE: nk_image(ctx, media->mage);	  break;
			case ASSASSIN: nk_image(ctx, media->assasin); break;
			case WARRIOR: nk_image(ctx, media->warrior); break;
			case KING: nk_image(ctx, media->king);	  break;
			}

			// username & points
			nk_text(ctx, ordered_usernames[i].c_str(), strlen(ordered_usernames[i].c_str()), NK_TEXT_LEFT);
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT);
			string player_d = std::to_string(deaths[i]);
			const char * player_death = player_d.c_str();
			nk_text(ctx, player_death, strlen(player_death), NK_TEXT_LEFT);
		}
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row(ctx, NK_DYNAMIC, height*0.12, 2, subTitleratio);
		nk_spacing(ctx, 1);
		nk_label(ctx, "Overall Summary", NK_TEXT_LEFT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
		nk_layout_row(ctx, NK_DYNAMIC, width*0.02, 6, globalLBratio);
		nk_spacing(ctx, 1);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, "Name", NK_TEXT_LEFT);
		nk_label(ctx, "Gold", NK_TEXT_LEFT);
		nk_label(ctx, "Points", NK_TEXT_LEFT);
		for (int i = 0; i < GAME_SIZE; i++) {

			const char * player_id;
			string s = std::to_string(i + 1);
			player_id = s.c_str();
			const char * player_point;

			// total points for player
			string point_s = std::to_string(ordered_points_global[i]);
			player_point = point_s.c_str();

			// total gold for player
			string gold_s = std::to_string(ordered_gold_global[i]);
			const char * player_gold = gold_s.c_str();

			nk_layout_row(ctx, NK_DYNAMIC, width*0.025, 6, globalLBratio);
			nk_spacing(ctx, 1);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);

			switch (ordered_types_global[i])	// archetype icon on leaderboard
			{
			case MAGE: nk_image(ctx, media->mage);	  break;
			case ASSASSIN: nk_image(ctx, media->assasin); break;
			case WARRIOR: nk_image(ctx, media->warrior); break;
			case KING: nk_image(ctx, media->king);	  break;
			}

			// TODO: SWITCH OUT WITH REAL SUMMARY PACKETS
			nk_text(ctx, ordered_usernames_global[i].c_str(), strlen(ordered_usernames_global[i].c_str()), NK_TEXT_LEFT);
			nk_text(ctx, player_gold, strlen(player_gold), NK_TEXT_LEFT);   // gold
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT); // points
		}

	}
	nk_end(ctx);
	if (nk_begin(ctx, "SwitchPage", nk_rect(0.03*width, 0.9*height, height*0.1, height*0.1),
		NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_static(ctx, height*0.1, height*0.1, 1);
		if (nk_button_image(ctx, media->right_arrow)) {
			Window_static::playButtonPress();
			guiS.currPrepareLayout = 1;
		}
	}
	nk_end(ctx);
}


static void ui_shop_header(struct nk_context *ctx, struct media *media, int width, int height,
	ScenePlayer * player, LeaderBoard * leaderBoard, ClientGame * game) {
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	if (nk_begin(ctx, "kill_header", nk_rect(5, height*0.15f, width * 0.19f, 190),
		NK_WINDOW_NO_SCROLLBAR))
	{
		static const float kill_ratio[] = { 0.3f,0.3f, 0.4f };  /* 0.3 + 0.4 + 0.3 = 1 */
		string roundStr = "NEXT ROUND: " + std::to_string(game->round_number);
		const char * round_char = roundStr.c_str();

		string goldStr = std::to_string(player->gold);
		string vicPtsStr = std::to_string(leaderBoard->currPoints[player->player_id]);
		const char * gold_char = goldStr.c_str();
		const char * vic_char = vicPtsStr.c_str();
		nk_style_set_font(ctx, &(media->font_48->handle));
		nk_layout_row_dynamic(ctx, 50, 1);
		nk_label(ctx, round_char, NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));

		nk_layout_row(ctx, NK_DYNAMIC, 116, 3, kill_ratio);

		nk_spacing(ctx, 1);
		if (nk_group_begin(ctx, "icons", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_image(ctx, media->gold);
			nk_layout_row_static(ctx, 20, 1, 1);
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_image(ctx, media->points);
		}
		nk_group_end(ctx);

		if (nk_group_begin(ctx, "nums", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_label(ctx, gold_char, NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
			nk_layout_row_static(ctx, 20, 1, 1);
			nk_layout_row_static(ctx, 36, 36, 1);
			nk_text(ctx, vic_char, strlen(vic_char), NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
		}
		nk_group_end(ctx);
	}
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}

static void 
ui_skill_group(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player,
	vector<int> prices, int row, char * name) {
	ArcheType type = player->modelType;
	static const float skratio[] = { 0.38f, 0.24f, 0.38f };  /* 0.3 + 0.4 + 0.3 = 1 */
	static const unsigned int sequential_bindings[] = {  OMNI_SKILL_INDEX, DIR_SKILL_INDEX, PROJ_INDEX, EVADE_INDEX };
	nk_layout_row_dynamic(ctx, height*0.23f + 160,1);

	if (nk_group_begin(ctx, name, NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row(ctx, NK_DYNAMIC, height*0.22f +160, 3, skratio);
		for (int j = 0; j < 3; j++) {
			int i = j / 2 + row*2;
			if (j % 2 == 0) {
				Skill & skill = player->availableSkills[i];
				const char * skill_string = skill.skillName.c_str();
				string p = "Cost: " + to_string(prices[i]);
				const char * price = p.c_str();
				if (nk_group_begin(ctx, skill_string, NK_WINDOW_NO_SCROLLBAR)) { // column 1
					nk_layout_row_static(ctx, height*0.15, height*0.15, 1); // nested row
					if (type == WARRIOR) {
						nk_image(ctx, media->warrior_skills[sequential_bindings[i]]);
					}
					else if (type == MAGE) {
						nk_image(ctx, media->mage_skills[sequential_bindings[i]]);
					}
					else if (type == ASSASSIN) {
						nk_image(ctx, media->assassin_skills[sequential_bindings[i]]);
					}
					else {
						nk_image(ctx, media->king_skills[sequential_bindings[i]]);
					}
					nk_layout_row_dynamic(ctx, 32, 1);
					nk_label(ctx, skill_string, NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 32, 1);
					nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 32, 1);
					nk_label(ctx, price, NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 32, 1);
					if (skill.level < 3) {
						string level = "Current Level: " + to_string(skill.level);
						nk_label(ctx, level.c_str(), NK_TEXT_ALIGN_LEFT);
					}
					else {
						nk_label(ctx, "MAX LEVEL", NK_TEXT_ALIGN_LEFT);
					}
					nk_layout_row_dynamic(ctx, 48, 1);
					if (skill.level < 3) {
						if (player->gold >= prices[i]) {
							if (nk_button_label(ctx, "upgrade")) {
								//Gold check and max level check
								player->gold -= prices[i];
								skill.level++;
								Window_static::playChaching();
							}
						}
						else {
							if (nk_button_label(ctx, "NOT ENOUGH GOLD")) {
								Window_static::playInvalidButtonPress();
							}
						}
					}
					else {
						if (nk_button_label(ctx, "NOT AVAILABLE")) {
							Window_static::playInvalidButtonPress();
						};
					}
					
				}
				nk_group_end(ctx);
			}
			else {
				nk_spacing(ctx, 1);
			}

		}
	}
	nk_group_end(ctx);
}


/*
	Scale cost of skill based on skill level.
*/
vector<int> scaleSkillCost(ScenePlayer* player)
{
	vector<int> prices;
	for (int i = 0; i < player->availableSkills.size(); i++)
	{
		// get current skill & ajdust gold based on level
		Skill curr_skill = player->availableSkills[i];	
		switch (curr_skill.level)
		{
			case 1: prices.push_back(10); break;
			case 2: prices.push_back(15); break;
			case 3: prices.push_back(20); break;
			default: prices.push_back(1000); break; // shouldn't happen
		}
	}

	return prices;
}

static void ui_skills_shop(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, ClientGame * game) {
	char* skill_string[4] = { "Cone AOE", "AOE", "Evade", "Projectile" };
	vector<int> prices = scaleSkillCost(player);

	//ArcheType type = player->modelType;
	if (nk_group_begin(ctx, "skill", NK_WINDOW_NO_SCROLLBAR)) {
		ui_skill_group(ctx, media, width, height, player, prices, 0, "row 0");
		ui_skill_group(ctx, media, width, height, player, prices, 1, "row 1");
	}
	nk_group_end(ctx);
}

static void ui_bets_shop(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, ClientGame * game, guiStatus & guiS) {
	char* skill_string[4] = { "Cone AOE", "AOE", "Evade", "Projectile" };
	static const char * characterTypeStrings[] = { "HUMAN", "MAGE", "ASSASSIN","WARRIOR","KING" };
	char* prices[4] = { "Cost: 5", "Cost: 10", "Cost: 15", "Cost: 20" };
	static const float bet_ratio[] = { 0.25f, 0.25f, 0.25f, 0.25f };
	static int op = -1;
	ArcheType type = player->modelType;
	if (nk_group_begin(ctx, "bets_shop", NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_static(ctx, height *0.12f, height *0.12f, 1); // nested row
		nk_layout_row(ctx, NK_DYNAMIC, width *0.13f +100, 4, bet_ratio);
		for (int i = 1; i < 5; i++) {
			//if (i == type) continue;
			if (nk_group_begin(ctx, characterTypeStrings[i], 0)) { // column 1
				nk_layout_row_static(ctx, width *0.12f, width *0.12f, 1); // nested row

				if (i == WARRIOR)
					nk_image(ctx, media->warrior);
				else if (i == MAGE)
					nk_image(ctx, media->mage);
				else if (i == ASSASSIN)
					nk_image(ctx, media->assasin);
				else
					nk_image(ctx, media->king);
				nk_layout_row_dynamic(ctx,70, 1);
				if (nk_option_label(ctx, characterTypeStrings[i], op == i)) op = static_cast<ArcheType>(i);

			}
			nk_group_end(ctx);
		}
		const char * amount;
		string s = "You are betting: " + std::to_string(guiS.betAmount) + " gold";
		amount = s.c_str();
		nk_layout_row_static(ctx, 48, 0.6*width, 1);
		nk_label(ctx, amount, NK_TEXT_CENTERED);
		nk_layout_row(ctx, NK_DYNAMIC, 65, 4, bet_ratio);
		nk_spacing(ctx, 1);
		nk_slider_int(ctx, 0, &guiS.betAmount, player->gold, 1);
		nk_spacing(ctx, 1);
		nk_layout_row(ctx, NK_DYNAMIC, height*0.05f, 4, bet_ratio);
		nk_spacing(ctx, 1);
		if (guiS.betAmount > 0 && nk_button_label(ctx, "Bet!")) {
			Window_static::playInvest();
			//TODO: SEND BET
			if (op >= 1) {
				player->amount_invested = guiS.betAmount;
				player->player_invested_in = static_cast<ArcheType>(op);
				player->gold -= guiS.betAmount;
			}
		}
		nk_spacing(ctx, 1);
	
		}
		nk_group_end(ctx);
}

static void ui_cheat_shop(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, ClientGame * game) {
	if (nk_group_begin(ctx, "cheat", NK_WINDOW_NO_SCROLLBAR)) {
		static const float cheatRatio[] = { 0.25f, 0.25f,  0.20f, 0.30f };
		static const float middeRatio[] = { 0.33f, 0.33f, 0.34f };
		nk_layout_row_static(ctx, 0.1*height, 0.3*width, 1);
		nk_layout_row(ctx, NK_DYNAMIC, 0.13*width, 4, cheatRatio);
		nk_spacing(ctx, 1);
		string cheating_point = to_string(CHEATING_POINTS) + " Point";
		nk_label(ctx, cheating_point.c_str(), NK_TEXT_CENTERED);
		nk_image(ctx, media->points);
		nk_spacing(ctx, 1);
		nk_layout_row(ctx, NK_DYNAMIC, height*0.03f, 3, middeRatio);
		nk_spacing(ctx, 1);
		string cheating_gold = "Cost: " + to_string(CHEATING_GOLD) + " gold";
		nk_label(ctx, cheating_gold.c_str(), NK_TEXT_CENTERED);
		nk_spacing(ctx, 1);
		nk_layout_row(ctx, NK_DYNAMIC, height*0.04f, 3, middeRatio);
		nk_spacing(ctx, 1);
		if (player->gold >= CHEATING_GOLD) {
			if (nk_button_label(ctx, "Buy Points!")) {
				game->cheatingPoints += CHEATING_POINTS;
				player->gold -= CHEATING_GOLD;
				Window_static::playChaching();
			}
		}
		else {
			if ((nk_button_label(ctx, "NOT ENOUGH GOLD"))) {
				Window_static::playInvalidButtonPress();
			}
		}
		nk_spacing(ctx, 1);

	}
	nk_group_end(ctx);
}

static void ui_shop(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, ClientGame * game, guiStatus & gStatuses) {

	if (nk_begin(ctx, "Shop_phase", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND
	))
	{
		static const float ratio[] = { 0.35f, 0.3f, 0.35f };  /* 0.3 + 0.4 + 0.3 = 1 */


		static const float choice_ratio[] = { 0.20f, 0.15f, 0.65f };
		ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
		ui_prepare_title(ctx, media, width, height, "Shop", game);

		nk_layout_row(ctx, NK_DYNAMIC, height*0.8, 3, choice_ratio);
		nk_spacing(ctx, 1);
		if (nk_group_begin(ctx, "shop_selections", NK_WINDOW_NO_SCROLLBAR)) { // column 1

			nk_layout_row_dynamic(ctx, 40, 1);
			if (nk_button_label(ctx, "Skills")) {
				Window_static::playButtonPress();
				gStatuses.shopCategory = 0;
			}
			nk_layout_row_static(ctx, 30, 1, 1);
			nk_layout_row_dynamic(ctx, 40, 1);
			if (nk_button_label(ctx, "Bet")) {
				Window_static::playButtonPress();
				gStatuses.shopCategory = 1;
			}
			nk_layout_row_static(ctx, 30, 1, 1);
			nk_layout_row_dynamic(ctx, 40, 1);
			if (nk_button_label(ctx, "Buy Points!")) {
				Window_static::playButtonPress();
				gStatuses.shopCategory = 2;
			}
		}
		nk_group_end(ctx);

		if (gStatuses.shopCategory == 0) {
			ui_skills_shop(ctx, media, width, height, player, game);
		}
		else if (gStatuses.shopCategory == 1) {
			ui_bets_shop(ctx, media, width, height, player, game, gStatuses);
		}
		else {
			ui_cheat_shop(ctx, media, width, height, player, game);
		}


	}
	nk_end(ctx);

	if (nk_begin(ctx, "SwitchPageBack", nk_rect(0.03*width, 0.9*height, height*0.1, height*0.1),
		NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_static(ctx, height*0.1, height*0.1, 1);
		if (nk_button_image(ctx, media->left_arrow)) {
			Window_static::playButtonPress();
			gStatuses.currPrepareLayout = 0;
		}
	}
	nk_end(ctx);
}

static  void
prepare_layout(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes, ClientGame * game, guiStatus & gStatuses) {
	
	set_style(ctx, THEME_BLACK);
	ctx->style.text.color = nk_rgba(235, 255, 235, 255);
	ctx->style.button.text_normal = nk_rgba(235, 255, 235, 255);
	ctx->style.window.fixed_background = nk_style_item_image(media->prepare_background);
	if (gStatuses.currPrepareLayout == 0) {
		ui_round_results(ctx, media, leaderBoard, usernames, archetypes, width, height, game, gStatuses);
	}
	else {
		ui_shop_header(ctx, media, width, height,  player, leaderBoard, game);
		ui_shop(ctx, media, width, height, player, game, gStatuses);
	}
}