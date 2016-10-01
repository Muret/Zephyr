#ifndef _WORLD_H
#define _WORLD_H

#include "includes.h"

namespace Evolution
{
	class Pop;
	class Need;
	class Action;
	class Tile;
	class WorldInstance;

	class KnowledgeHistory
	{
	public:
		KnowledgeHistory();

		void add_knowledge(const std::string &name, float amount);
		float get_knowledge(const std::string &name) const;

	private:
		std::map<std::string, float> knowledge_set_;
	};

	class Action
	{
	public:
		Action(string name);

		virtual int execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop &executer) = 0;

	private:
		string name_;
	};

	class TravelAction : public Action
	{
	public:
		TravelAction() : Action("Travel") {}

		virtual int execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop &executer) override;
	};

	class ObserveAction : public Action
	{
	public:
		ObserveAction() : Action("Observe") {}
		
		virtual int execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop &executer) override;
	};

	class HuntAction : public Action
	{
	public:
		HuntAction() : Action("Hunt") {}

		virtual int execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop &executer) override;
	};

	class Need
	{
	public:
		Need(float proiority, string name);

	private:
		float prioirty_;
		string name_;

	};

	class Animal
	{
	public:
		Animal( string name, initializer_list<pair<string,float>> plain_item_yield, float hunt_difficulty);

		float get_item_yield(const string &name) const;
		string get_name() const;
		float get_hunt_difficulty() const;
		void get_total_yield(int number_of_catches, vector<pair<string, float>> &total_yield) const;

	private:
		string name_;
		map<string, float> plain_item_yield_;
		float hunt_difficulty_;
	};

	class Plant
	{
	public:
		Plant(string name, initializer_list<pair<string, float>> plain_item_yield, float gather_difficulty);

	private:
		string name_;
		map<string, float> plain_item_yield_;
		float gather_difficulty_;
	};

	class Pop
	{
	public:
		Pop();

		void tick();
		void decide();
		void give_plain_items(vector<pair<string, float>> items);

		void set_current_tile(int index);
		void add_knowledge(string name, float amount);

		int get_current_tile() const;
		const KnowledgeHistory& get_knowledge() const;

	private:
		int pop_count_;
		float health_value_;

		int current_tile_;

		KnowledgeHistory knowledge_;

		std::vector<int> cur_action_indices_;
		std::map<string, float> plain_items_owned_;

	};

	class Tile
	{
	public:

		void reduce_animal_pop(int animal_id, int number);

		int get_animal_population(int animal_index) const;
	private:
		D3DXVECTOR3 center;
		std::vector<int> neightbors;
	};

	class WorldInstance
	{
	public:
		WorldInstance(int seed);

		float get_random_value();
		float get_distance_between_tiles(int first_tile, int second_tile);

		Tile *get_tile(int tile_index) const;
		const Animal &get_animal(int animal_index) const;

	private:
		vector<Action*> actions_;
		vector<Need> needs_;
		vector<string> plain_items_;
		vector<Animal> animals_;
		vector<Plant> plants_;
	};

	extern WorldInstance *g_world_instance_;

};

#endif