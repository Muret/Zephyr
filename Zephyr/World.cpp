
#include "World.h"

using namespace Evolution;

namespace Evolution
{
	//--TODO_LIST
	// execute functions should take argument for hours spent (-1 for enough)
	// implement AI (decide function) , there should be primary and secondary jobs
	// implement ticks , daily(food consumption) and weekly(health, pop and exp reduce)
	// fill tiles with pop counts, weekly ticks for animal pop
	// gather action ?
	// usable item implementation and stone age..
	//-----------

	WorldInstance *g_world_instance_;

	WorldInstance::WorldInstance(int seed)
	{
		{
			needs_.push_back(Need(1, "hunger"));
		}

		{
			actions_.push_back(new TravelAction());
			actions_.push_back(new ObserveAction());
		}

		//plain items
		{
			plain_items_.push_back("meat");
			plain_items_.push_back("stone");
			plain_items_.push_back("wood");
			plain_items_.push_back("wool");
			plain_items_.push_back("hide");
		}

		//animals
		{
			animals_.push_back(Animal("sheep", { make_pair("food",0.5) , make_pair("wool",0.4), make_pair("hide",0.1) }, 0.3));
			animals_.push_back(Animal("deer", { make_pair("food",0.9) , make_pair("hide",0.3) }, 0.6));
		}

		//plants
		{
			plants_.push_back(Plant("berry", { make_pair("food",0.1) }, 0.1));
		}


		srand(seed);
	}

	float WorldInstance::get_random_value()
	{
		return float(rand() % RAND_MAX) / (float)RAND_MAX;
	}

	float WorldInstance::get_distance_between_tiles(int first_tile, int second_tile)
	{
		//TODO_MURAT : implement
		return 0.0f;
	}

	void Tile::reduce_animal_pop(int animal_id, int number)
	{
		//TODO_MURAT : implement
	}

	int Tile::get_animal_population(int animal_index) const
	{
		//TODO_MURAT : implement
		return 0;
	}

	Tile * WorldInstance::get_tile(int tile_index) const
	{
		//TODO_MURAT : implement
		return nullptr;
	}

	const Animal& WorldInstance::get_animal(int animal_index) const
	{
		if (animal_index >= animals_.size())
		{
			_ASSERT(false);
			return animals_[0];
		}

		return animals_[animal_index];
	}

	float Animal::get_item_yield(const string & name) const
	{
		auto it = plain_item_yield_.find(name);
		if (it != plain_item_yield_.end())
		{
			return it->second;
		}

		return 0.0f;
	}

	string Animal::get_name() const
	{
		return name_;
	}

	float Animal::get_hunt_difficulty() const
	{
		return hunt_difficulty_;
	}

	void Animal::get_total_yield(int number_of_catches, vector<pair<string, float>>& total_yield) const
	{
		for (auto it = plain_item_yield_.begin(); it != plain_item_yield_.end(); it++)
		{
			total_yield.push_back(make_pair(it->first, it->second * number_of_catches));
		}

	}

	Action::Action(string name)
	{
		name_ = name;
	}

	Need::Need(float prioirty, string name)
	{
		prioirty_ = prioirty;
		name_ = name;
	}

	int TravelAction::execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop & executer)
	{
		int current_tile = executer.get_current_tile();
		int next_tile = int_parameters[0];

		float distance = g_world_instance_->get_distance_between_tiles(current_tile, next_tile);

		_ASSERT(distance > 0);

		//TODO_MURAT : side effects of travel

		executer.set_current_tile(next_tile);
	}

	int ObserveAction::execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop & executer)
	{
		//TODO_MURAT : handle the effect of "amount of things observed"

		string observed_thing = string_parameters[0];
		
		const float max_exp_gained = 0.001f;
		float exp_gained = g_world_instance_->get_random_value() * max_exp_gained;

		executer.add_knowledge(observed_thing, exp_gained);
	}

	int HuntAction::execute(std::vector<int> int_parameters, std::vector<string> string_parameters, Pop & executer)
	{
		int hunted_animal_id = int_parameters[0];
		int meat_amount_to_hunt_for = int_parameters[1];
		
		Tile *cur_tile = g_world_instance_->get_tile(executer.get_current_tile());
		int cur_tile_animal_pop = cur_tile->get_animal_population(hunted_animal_id);

		const Animal &animal = g_world_instance_->get_animal(hunted_animal_id);
		float meat_yield_of_animal = animal.get_item_yield("meat");

		int desired_hunt_count = ((float)meat_amount_to_hunt_for / meat_yield_of_animal) + 1;

		//hunt efficency of pop
		string name("hunt");
		float general_hunt_exp = executer.get_knowledge().get_knowledge(name);
		float spcific_animal_hunt_exp = executer.get_knowledge().get_knowledge(name + animal.get_name());
		float cur_animal_hunt_difficulty = animal.get_hunt_difficulty();

		float total_hunting_exp_factor = general_hunt_exp + spcific_animal_hunt_exp;

		//TODO_MURAT : hunter amount ?
		float hours_in_day = 16.0f;
		float base_hunt_time_per_animal = 16.f * cur_animal_hunt_difficulty;
		float hunt_time_per_animal = base_hunt_time_per_animal * (1.0f - max(total_hunting_exp_factor, 2.0f) * 0.2f);
		
		float chance_of_succes = 0.2f + 0.2 *  max(total_hunting_exp_factor, 4.0f);
		float total_exp_gained = 0.0f;
		int number_of_catches = 0;

		float time_spent = 0.0f;
		while (1)
		{
			float value = g_world_instance_->get_random_value();
			if (value < chance_of_succes)
			{
				number_of_catches++;
				total_exp_gained += 0.001;
			}
			
			total_exp_gained += 0.001;
			time_spent += hunt_time_per_animal;

			if (number_of_catches >= desired_hunt_count || time_spent > hours_in_day)
			{
				break;
			}
		}

		if (number_of_catches > 0)
		{
			vector<pair<string, float>> total_yield;
			animal.get_total_yield(number_of_catches, total_yield);
			executer.give_plain_items(total_yield);

			cur_tile->reduce_animal_pop(hunted_animal_id, number_of_catches);
		}

		executer.add_knowledge(name, total_exp_gained * 0.5f);
		executer.add_knowledge(name + animal.get_name(), total_exp_gained);
	}

	KnowledgeHistory::KnowledgeHistory()
	{

	}

	void KnowledgeHistory::add_knowledge(const std::string &name, float amount)
	{
		auto it = knowledge_set_.find(name);
		if (it != knowledge_set_.end())
		{
			it->second += amount;
		}
		else
		{
			it->second = amount;
		}
	}

	float KnowledgeHistory::get_knowledge(const std::string & name) const
	{
		auto it = knowledge_set_.find(name);
		if (it != knowledge_set_.end())
		{
			return it->second;
		}

		return 0.0f;
	}

	Plant::Plant(string name, initializer_list<pair<string, float>> plain_item_yield, float gather_difficulty)
	{
		for (auto it = plain_item_yield.begin(); it != plain_item_yield.end(); it++) 
		{
			plain_item_yield_[it->first] = it->second;
		}

		name_ = name;
		gather_difficulty_ = gather_difficulty;
	}

	Animal::Animal(string name, initializer_list<pair<string, float>> plain_item_yield, float hunt_difficulty)
	{
		for (auto it = plain_item_yield.begin(); it != plain_item_yield.end(); it++)
		{
			plain_item_yield_[it->first] = it->second;
		}

		name_ = name;
		hunt_difficulty = hunt_difficulty_;
	}



	Pop::Pop()
	{
	}

	void Pop::tick()
	{
	}

	void Pop::decide()
	{
	}

	void Pop::give_plain_items(vector<pair<string, float>> items)
	{
		for (int i = 0; i < items.size(); i++)
		{
			auto it = plain_items_owned_.find(items[i].first);
			if (it != plain_items_owned_.end())
			{
				it->second += items[i].second;
			}
			else
			{
				it->second = items[i].second;
			}
		}
	}

	void Pop::set_current_tile(int index)
	{
		current_tile_ = index;
	}

	void Pop::add_knowledge(string name, float amount)
	{
		knowledge_.add_knowledge(name, amount);
	}

	int Pop::get_current_tile() const
	{
		return current_tile_;
	}

	const KnowledgeHistory & Pop::get_knowledge() const
	{
		return knowledge_;
	}

}
