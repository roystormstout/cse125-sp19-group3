#pragma once
#include "CoreTypes.hpp"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <algorithm>    // std::sort
#include <vector>
#include <list>
#include "INIReader.h"

using namespace std;

class LeaderBoard {
public:

	// default constructor
	LeaderBoard() : currentKills(vector<int>(GAME_SIZE, 0)), currentDeaths(vector<int>(GAME_SIZE,0)),
					killStreaks(vector<int>(GAME_SIZE, 0)), globalKills(vector<int>(GAME_SIZE,0)),
					currPoints(vector<int>(GAME_SIZE, 0)), currGold(vector<int>(GAME_SIZE,0)),
					prizes(vector<int>(GAME_SIZE, 0)), deaths_this_tick(0) {}

	~LeaderBoard() {}

	//vector<int>* roundSummary ();						// Update vectors for the round and return the ranking of each player
	void awardKillRound(unsigned int player_id);		// award point to player_id on rounds leaderboard
	void awardKillGlobal(unsigned int player_id);		// award point to player_id on global leaderboard
	void resetKillStreak(unsigned int player_id);		// reset players kill streak
	void incKillStreak(unsigned int player_id);			// increment players killstreak
	void incDeath(unsigned int player_id);				// increment players death count
	void awardPoint(unsigned int player_id);			// award point to player_id
	void awardRoundPoints(int round_number);			// award points to all players based on rank

	// get winner(s) of last round
	vector<ArcheType> getRoundWinner(unordered_map<ArcheType, int>* selected_characters);

	// for testing
	void printCurrentKillStreaks();	
	void printCurrentKills();	
	void printCurrPoints();	
	void printDeathCount();	
	void printPrizes();	

	// RESET THESE VECTORS THREE VECTORS AFTER EVERY ROUND
	vector<int> currentKills;	// current rounds kills for each player
	vector<int> currentDeaths;  // current rounds deaths for each player
	vector<int> killStreaks;	// current rounds killstreaks for each player

	vector<int> globalKills;	// total # of kills in all rounds
	vector<int> currPoints;		// accumulative points of each player
	vector<int> currGold;		// accumulative gold of each player

	list<int> curr_killstreaks;	// track which players are on killstreaks
	list<int> curr_shutdowns;	// trakc which players shutdown other players

	vector<int> prizes;			// points added to player each round based on ranking
	list<int> kill_map;			// map of who killed who on every server tick
	int deaths_this_tick;		// total deaths that occured this tick
	int total_killstreaks = 0;  // total killstreaks per tick 
	int total_shutdowns = 0;    // total shutdowns per tick
};

class Skill {
public:
	string skillName;
	unsigned int level;
	unsigned int skill_id;
	float range;
	int cooldown; // in milliseconds
	int duration; // in milliseconds
	float speed;
	Skill(unsigned int skill_id, unsigned int initialLevel, string skillName, float range, int cooldown, int duration, float speed) {
		this->skillName = skillName;
		this->level = initialLevel;
		this->skill_id = skill_id;
		this->range = range;
		this->cooldown = cooldown;
		this->duration = duration;
		this->speed = speed;
	};

	Skill() : skillName("Default Skill"), range(-1), cooldown(-1), duration(-1), speed(-1) {}
	~Skill(){}
	static void load_archtype_data(unordered_map<unsigned int, Skill> *skill_map, 
		                           unordered_map<ArcheType, vector<unsigned int>> *archetype_skill_set);
	static Skill calculateSkillBasedOnLevel(Skill &baseSkill, unsigned int level);
};

/*
	Stores players metadata (id, name, type, gold, killstreak, death status).
*/
class PlayerMetadata {
public:
	PlayerMetadata(unsigned int clientId, 
		           std::string username, 
		           ArcheType type, 
		           unordered_map<unsigned int, Skill> *skill_map, 
		           unordered_map<ArcheType, vector<unsigned int>> *archetype_skillsets) : clientId(clientId), username(username), type(type) 
{

		// get vector of skillsets from archetype map
		unordered_map<ArcheType, vector<unsigned int>>::iterator a_it = archetype_skillsets->find(type);
		vector<unsigned int> vec = a_it->second;		

		for (auto skill_id : vec) {
			//Skill_Map::iterator s_it = skill_map->find(x); // Question: Why doesnt this typedef work?!
			unordered_map<unsigned int, Skill>::iterator s_it = skill_map->find(skill_id);
			auto level = s_it->second.level;
			skillLevels.insert({ skill_id, level });
		}

		alive = true;
		silenced = false;
		gold = 0;
		currKillStreak = 0;
		currLoseStreak = 0;
		
	}
	PlayerMetadata() {};
	~PlayerMetadata() {};

	unsigned int clientId;
	std::string username;
	ArcheType type;
	unordered_map<unsigned int, unsigned int> skillLevels;

	bool alive;
	bool silenced;

	// TODO: currently not plan to implement gold, purchase
	// TODO: how to implement weapons
	int gold;
	int currKillStreak;		// to give out gold
	int currLoseStreak;		// to give out gold
	unordered_set<string> inventory;	// items from shop
};

