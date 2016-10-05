
#include "World.h"
#include "ResourceManager.h"
#include "GUI.h"
#include "Utilities.h"

namespace Evolution
{
	//--TODO_LIST
	// (DONE)execute functions should take argument for hours spent (-1 for enough)
	// (DONE)implement AI (decide function) , there should be primary and secondary jobs
	// (DONE)implement ticks , daily(food consumption) and weekly(health, pop and exp reduce)
	// (DONE)fill tiles with pop counts
	// (DONE)pop reproduction
	// (DONE)start ticking the first game
	// (DONE)re-implement hunting efficency
	// (DONE)convert to weakly ticks and ai
	// (DONE)free time concept and weighted random for free time orderings and percentages
	// gathering rocks, woods
	// knowledge discover concept , crafting items concept and crafting item act
	// item efficency for actions and item depletion when used
	// animal knowledge for yield of animal
	// pop count and efficency for hunting
	// weapons 
	// global variable for days per tick
	// trees, plants , gathering
	// knowledge pheomena
	// gather action ?
	//-----------

	WorldInstance *g_world_instance_;

	WorldInstance::WorldInstance(int seed, const std::vector<VoronoiSolver::VoronoiSite> &voronoi_cells) : days_per_tick_(7)
	{
		g_world_instance_ = this;

		srand(seed);
		last_index_ = -1;
		
		//plain items
		{
			new PlainItem("food" );
			new PlainItem("stone");
			new PlainItem("wood" );
			new PlainItem("wool" );
			new PlainItem("hide" );
		}

		//animals
		{
			new Animal("sheep", { make_pair("food",0.5) , make_pair("wool",0.4), make_pair("hide",0.1) }, 0.3);
			new Animal("deer", { make_pair("food",0.9) , make_pair("hide",0.3) }, 0.6);
		}

		//plants
		{
			new Plant("berry", { make_pair("food",0.1) }, 0.1);
		}

		//actions
		{
			new TravelAction();
			new ObserveAction();
			new HuntAction();
		}

		vector<Need*> needs;

		//need
		{
			needs.push_back(new Need("hunger", { make_pair("food", 0.03 * days_per_tick_) } , 1.0f));
		}

		geography_ = new Geography(voronoi_cells);

		static const int number_of_pops = 1000;
		int tile_count = geography_->get_number_of_tiles();
		for (int i = 0; i < number_of_pops; i++)
		{
			Tile *tile_to_spawn = nullptr;
			int tile_to_spawn_index = -1;
			while (true)
			{
				int index_cand = g_world_instance_->get_random_value() * float(tile_count - 1);
				Tile *tile_cand = geography_->get_tile(index_cand);
				if (tile_cand && tile_cand->get_climate()->get_name() != "Sea")
				{
					tile_to_spawn = tile_cand;
					tile_to_spawn_index = index_cand;
					break;
				}
			}

			int base_pop_spawn_count = 25;
			float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;

			Pop *new_pop = new Pop(base_pop_spawn_count * percent, pops_.size());
			new_pop->set_current_tile(tile_to_spawn_index);
			new_pop->set_needs(needs);
			pops_.push_back(new_pop);
		}

		automatic_tick_ = false;
	}

	float WorldInstance::get_random_value()
	{
		return float(rand() % RAND_MAX) / (float)RAND_MAX;
	}

	float WorldInstance::get_distance_between_tiles(int first_tile, int second_tile)
	{
		D3DXVECTOR3 first_tile_position = geography_->get_tile(first_tile)->get_position();
		D3DXVECTOR3 second_tile_position = geography_->get_tile(second_tile)->get_position();

		D3DXVECTOR3 diff = second_tile_position - first_tile_position;

		return sqrt(D3DXVec3Dot(&diff, &diff));
	}

	Tile::Tile(const D3DXVECTOR3 & center, int tile_index, vector<int> neighbours, Climate * climate) : ObjectDefition(Utilities::formatted_string("tile%d", tile_index), ObjectDefition::Type::tile)
	{
		center_ = center;
		tile_index_ = tile_index;
		neighbours_ = neighbours;
		climate_ = climate;

		const map<int, int>& animals = climate->get_animal_pop();
		for (auto it = animals.begin(); it != animals.end(); it++)
		{
			float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;
			animal_pop_[it->first] = percent * it->second;
		}

		const map<int, int>& plants = climate->get_fauna();
		for (auto it = plants.begin(); it != plants.end(); it++)
		{
			float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;
			plant_pop_[it->first] = percent * it->second;
		}

		string knowledge_name = Utilities::formatted_string("tile%d_knowledge", tile_index);
		tile_knowledge_ = (new Knowledge(knowledge_name, ObjectDefition::Type::tile))->get_id();
	}

	void Tile::reduce_animal_pop(int animal_id, int number)
	{
		auto it = animal_pop_.find(animal_id);
		SAFEZEPHYR_ASSERT(it != animal_pop_.end())
		{
			it->second -= number;
			ZEPHYR_ASSERT(it->second >= 0);
		}
	}

	int Tile::get_animal_population(int animal_index) const
	{
		auto it = animal_pop_.find(animal_index);
		if (it != animal_pop_.end())
		{
			return it->second;
		}
		return 0;
	}

	void Tile::get_animal_ids_in_tile(vector<int>& animal_indeces) const
	{
		for (auto it = animal_pop_.begin(); it != animal_pop_.end(); it++)
		{
			if (it->second > 0)
			{
				animal_indeces.push_back(it->first);
			}
		}
	}

	void Tile::get_plant_ids_in_tile(vector<int>& plant_indeces) const
	{
		for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
		{
			if (it->second > 0)
			{
				plant_indeces.push_back(it->first);
			}
		}
	}

	void Tile::get_things_in_tile(vector<int>& things_in_tile)
	{
		get_animal_ids_in_tile(things_in_tile);
		get_plant_ids_in_tile(things_in_tile);
	}

	void Tile::render_gui()
	{
		ImGui::Begin("Tile Info");
		ImGui::Text("Climate : %s\n", climate_->get_name().c_str());

		if (ImGui::TreeNode("Animals"))
		{
			for (auto it = animal_pop_.begin(); it != animal_pop_.end(); it++)
			{
				ImGui::Text("Animal (%s) amount (%d)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Plants"))
		{
			for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
			{
				ImGui::Text("Plant (%s) amount (%d)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Pops"))
		{
			for (int i = 0; i < living_pops_.size(); i++)
			{
				ImGui::Text("Pop id-%d", living_pops_[i]);
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

	Tile * WorldInstance::get_tile(int tile_index) const
	{
		return geography_->get_tile(tile_index);
	}

	int WorldInstance::get_index()
	{
		last_index_++;
		return last_index_;
	}

	void WorldInstance::get_indexes_of_type(ObjectDefition::Type type, vector<int> &indexes)
	{
		for (int i = 0; i < object_definitions_.size(); i++)
		{
			if (object_definitions_[i]->get_type() == type)
			{
				indexes.push_back(object_definitions_[i]->get_id());
			}
		}
	}

	ObjectDefition * WorldInstance::get_object_by_name(string name) const
	{
		auto it = name_to_index_map_.find(name);
		if (it != name_to_index_map_.end())
		{
			int index = it->second;
			auto index_it = index_to_object_map_.find(index);
			if (index_it != index_to_object_map_.end())
			{
				return index_it->second;
			}
		}

		return nullptr;
	}

	int WorldInstance::get_random_neighbour_tile(int tile_index) const
	{
		Tile *tile = geography_->get_tile(tile_index);
		SAFEZEPHYR_ASSERT(tile != nullptr)
		{
			vector<int> neighbours = tile->get_neighbours();
			int neighbour_count = neighbours.size();

			SAFEZEPHYR_ASSERT(neighbour_count > 0)
			{
				int random_index = g_world_instance_->get_random_value() * (neighbour_count - 1);
				return neighbours[random_index];
			}
		}

		return -1;
	}

	void WorldInstance::register_object_definition(ObjectDefition *object)
	{
		auto it = name_to_index_map_.find(object->get_name());
		ZEPHYR_ASSERT(it == name_to_index_map_.end());

		name_to_index_map_[object->get_name()] = object->get_id();
		index_to_type_map_[object->get_id()] = object->get_type();
		index_to_object_map_[object->get_id()] = object;

		object_definitions_.push_back(object);
	}

	void WorldInstance::tick()
	{
		ImGui::Begin("World Simulation",nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Automatic Tick", &automatic_tick_);

		static int tick_count = 1;
		ImGui::InputInt("Tick Count", &tick_count);
		ImGui::SameLine();
		if (ImGui::Button("Single Tick"))
		{
			for (int i = 0; i < tick_count; i++)
			{
				tick_aux();
			}
		}

		static int selected_pop_index = 0;
		ImGui::InputInt("Pop index", &selected_pop_index);
		
		selected_pop_index = max(selected_pop_index, 0);
		selected_pop_index = min(selected_pop_index, pops_.size() - 1);

		static int selected_tile_index = 0;
		ImGui::InputInt("Tile Index", &selected_tile_index);

		ImGui::End();

		Tile *cur_tile = geography_->get_tile(selected_tile_index);
		if (cur_tile)
		{
			cur_tile->render_gui();
		}

		pops_[selected_pop_index]->render_gui();

		if (automatic_tick_)
		{
			tick_aux();
		}
	}

	void WorldInstance::toggle_tile_render()
	{
		tile_render_on_ = !tile_render_on_;
	}

	void WorldInstance::tick_aux()
	{
		for (int i = 0; i < pops_.size(); i++)
		{
			pops_[i]->decide();
		}

		for (int i = 0; i < pops_.size(); i++)
		{
			pops_[i]->tick();
		}
	}


	float Animal::get_item_yield(int index) const
	{
		auto it = plain_item_yield_.find(index);
		if (it != plain_item_yield_.end())
		{
			return it->second;
		}

		return 0.0f;
	}

	float Animal::get_hunt_difficulty() const
	{
		return hunt_difficulty_;
	}

	void Animal::get_total_yield(int number_of_catches, vector<pair<int, float>>& total_yield) const
	{
		for (auto it = plain_item_yield_.begin(); it != plain_item_yield_.end(); it++)
		{
			total_yield.push_back(make_pair(it->first, it->second * number_of_catches));
		}

	}

	Action::Action(string name) : ObjectDefition(name, ObjectDefition::Type::action)
	{
	}

	float TravelAction::execute(std::vector<int> int_parameters, Pop * executer, float spent_hours, string &log)
	{
		int current_tile = executer->get_current_tile();
		int next_tile = int_parameters[0];

		float distance = g_world_instance_->get_distance_between_tiles(current_tile, next_tile);

		ZEPHYR_ASSERT(distance > 0);

		//TODO_MURAT : side effects of travel, knowledge gain
		//TODO_MURAT : every travel finishes at one day ? 

		executer->set_current_tile(next_tile);

		log += Utilities::formatted_string("Moved from %d to %d\n", current_tile, next_tile);

		return 0.0f;
	}

	float ObserveAction::execute(std::vector<int> int_parameters, Pop * executer, float spent_days, string &log)
	{
		int thing_id = int_parameters[0];
		ObjectDefition *thing_to_observe = g_world_instance_->get_object_by_index(thing_id);
		int knowledge_id = thing_to_observe->get_associated_knowledge_id();

		Knowledge *knowledge = (Knowledge *)g_world_instance_->get_object_by_index(knowledge_id);
		ZEPHYR_ASSERT(knowledge->get_type() == ObjectDefition::Type::knowledge);

		static const float days_spent_to_maximize_skill = 12 * 30;
		static const float exp_gained_per_day = 1.0f / days_spent_to_maximize_skill;

		float exp_gained = (0.5 + g_world_instance_->get_random_value()) * exp_gained_per_day * spent_days;

		log += Utilities::formatted_string("Observe action with days spent (%f).\n", spent_days);

		if (thing_to_observe->get_type() == ObjectDefition::Type::tile)
		{
			Tile *cur_tile = (Tile *)thing_to_observe;
			vector<int> things_in_tile;
			cur_tile->get_things_in_tile(things_in_tile);

			if (things_in_tile.size() > 0)
			{
				int index = (things_in_tile.size() - 1) * g_world_instance_->get_random_value();
				int extra_knowledge_id = g_world_instance_->get_object_by_index(things_in_tile[index])->get_associated_knowledge_id();
				float extra_exp_gained = (0.5 + g_world_instance_->get_random_value()) * exp_gained_per_day * spent_days;
			
				executer->add_knowledge(extra_knowledge_id, extra_exp_gained);
				log += Utilities::formatted_string("Observed (%s) , gained (%f) exp.\n", g_world_instance_->get_object_by_index(extra_knowledge_id)->get_name().c_str(), extra_exp_gained);
			}

			exp_gained *= 0.75;
		}

		executer->add_knowledge(knowledge_id, exp_gained);

		log += Utilities::formatted_string("Observed (%s) , gained (%f) exp.\n", g_world_instance_->get_object_by_index(knowledge_id)->get_name().c_str(), exp_gained);

		return spent_days;
	}

	HuntAction::HuntAction() : Action("Hunt")
	{
		vector<int> animal_defs;
		g_world_instance_->get_indexes_of_type(ObjectDefition::Type::animal, animal_defs);
		global_hunting_knowledge_id_ = (new Knowledge("hunting_knowledge", ObjectDefition::Type::action))->get_id();

		for (int i = 0; i < animal_defs.size(); i++)
		{
			int animal_index = animal_defs[i];
			string name = g_world_instance_->get_object_by_index(animal_index)->get_name() + string("_hunting");
			int hunting_knowledge_index = (new Knowledge(name, ObjectDefition::Type::action))->get_id();
			animal_index_to_hunting_knowledge_index_.insert(make_pair(animal_index, hunting_knowledge_index));
		}
	}

	float HuntAction::execute(std::vector<int> int_parameters,Pop * executer, float spent_hours, string &log)
	{
		int hunted_animal_id = int_parameters[0];
		int desired_hunt_count = int_parameters[1];
		
		Tile *cur_tile = g_world_instance_->get_tile(executer->get_current_tile());
		int cur_tile_animal_pop = cur_tile->get_animal_population(hunted_animal_id);
		ZEPHYR_ASSERT(cur_tile_animal_pop >= desired_hunt_count);

		const Animal* animal = (const Animal*)g_world_instance_->get_object_by_index(hunted_animal_id);

		//calculate hunt efficency
		//base -> 0.3
		//hunting exp -> 0.25 at 1.0
		//animal information -> 0.25 at 1.0
		//animal hunt exp -> 0.3 at 1.0
		//weapon -> 0.4
		//geographic -> 0.5
		float base_efficency = 0.3f;
		float hunting_exp = executer->get_knowledge().get_knowledge(global_hunting_knowledge_id_) * 0.25;
		float animal_exp = executer->get_knowledge().get_knowledge(animal->get_animal_knowledge_index()) * 0.25;
		float animal_hunt_exp = executer->get_knowledge().get_knowledge( animal_index_to_hunting_knowledge_index_[hunted_animal_id] ) * 0.3;
		float weapon_factor = 0 * 0.4f;
		float geographic_factor = (1.0f - cur_tile->get_climate()->get_hunting_difficulty()) * 0.5;
		float cur_animal_hunt_difficulty = animal->get_hunt_difficulty();

		float total_efficency = base_efficency + hunting_exp + animal_exp + animal_hunt_exp + weapon_factor + geographic_factor;

		const float base_hunt_time_per_animal = 0.125f;
		float time_factor = total_efficency * (1.0f - cur_animal_hunt_difficulty) * (g_world_instance_->get_random_value() * 2.0f);
		float hunt_time = base_hunt_time_per_animal / time_factor;

		log += Utilities::formatted_string("Hunt total_efficency (%f), time multiplier (%f) , hunt time (%f).\n", total_efficency, 1.0f / time_factor, hunt_time);

		float total_days_spent = min((desired_hunt_count * hunt_time), g_world_instance_->get_days_per_tick());
		int number_of_catches = total_days_spent / hunt_time;

		log += Utilities::formatted_string("Spent hours (%f), wanted (%d), hunted #(%d) of (%s).\n", total_days_spent, desired_hunt_count, number_of_catches, animal->get_name().c_str());

		if (number_of_catches > 0 && cur_tile_animal_pop >= number_of_catches)
		{
			vector<pair<int, float>> total_yield;
			animal->get_total_yield(number_of_catches, total_yield);
			executer->give_plain_items(total_yield);

			for (auto it = total_yield.begin(); it != total_yield.end(); it++)
			{
				log += Utilities::formatted_string("Gained item(%s) of amount (%f).\n", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
			}

			cur_tile->reduce_animal_pop(hunted_animal_id, number_of_catches);
		}

		static const float days_spent_to_maximize_skill = 6 * 30;
		static const float exp_gained_per_day = 1.0f / days_spent_to_maximize_skill;

		executer->add_knowledge(animal->get_animal_knowledge_index(), exp_gained_per_day * 0.25f * total_days_spent);
		executer->add_knowledge(global_hunting_knowledge_id_, exp_gained_per_day * 0.5f * total_days_spent);
		executer->add_knowledge(animal_index_to_hunting_knowledge_index_[hunted_animal_id], exp_gained_per_day * total_days_spent);

		//log += Utilities::formatted_string("Gained exp(%s) of amount (%f).\n", g_world_instance_->get_object_by_index(animal->get_animal_knowledge_index())->get_name().c_str(), total_exp_gained * 0.5f);
		//log += Utilities::formatted_string("Gained exp(%s) of amount (%f).\n", g_world_instance_->get_object_by_index(animal_index_to_hunting_knowledge_index_[hunted_animal_id])->get_name().c_str(), total_exp_gained);

		return total_days_spent;
	}

	KnowledgeHistory::KnowledgeHistory()
	{

	}

	void KnowledgeHistory::add_knowledge(int index, float amount)
	{
		auto it = knowledge_set_.find(index);
		if (it != knowledge_set_.end())
		{
			it->second += amount;
		}
		else
		{
			knowledge_set_[index] = amount;
		}
	}

	float KnowledgeHistory::get_knowledge(int index) const
	{
		auto it = knowledge_set_.find(index);
		if (it != knowledge_set_.end())
		{
			return it->second;
		}

		return 0.0f;
	}

	void KnowledgeHistory::tick()
	{
		for (auto it = knowledge_set_.begin(); it != knowledge_set_.end(); it++)
		{
			it->second *= 0.98;
		}
	}

	void KnowledgeHistory::render_gui()
	{
		for (auto it = knowledge_set_.begin(); it != knowledge_set_.end(); it++)
		{
			if (it->second > 0)
			{
				ImGui::Text("Knowledge (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
			}
		}
	}

	Plant::Plant(string name, initializer_list<pair<string, float>> plain_item_yield, float gather_difficulty) : ObjectDefition(name,ObjectDefition::Type::plant)
	{
		for (auto it = plain_item_yield.begin(); it != plain_item_yield.end(); it++) 
		{
			plain_item_yield_[it->first] = it->second;
		}

		gather_difficulty_ = gather_difficulty;

		plant_knowledge_index_ = (new Knowledge(name + string("_plant"), ObjectDefition::Type::plant))->get_id();
	}

	Animal::Animal(string name, initializer_list<pair<string, float>> plain_item_yield, float hunt_difficulty) : ObjectDefition(name,ObjectDefition::Type::animal)
	{
		for (auto it = plain_item_yield.begin(); it != plain_item_yield.end(); it++)
		{
			int plain_item_index = g_world_instance_->get_object_by_name(it->first)->get_id();
			plain_item_yield_[plain_item_index] = it->second;
		}

		hunt_difficulty_ = hunt_difficulty;
		ZEPHYR_ASSERT(hunt_difficulty_ > 0);
		animal_knowledge_index_ = (new Knowledge(name + string("_animal"), ObjectDefition::Type::animal))->get_id();
	}

	Pop::Pop(int count, int pop_id)
	{
		pop_count_ = count;
		pop_id_ = pop_id;
	}

	void Pop::tick()
	{
		float days_remaining = g_world_instance_->get_days_per_tick();
		for (int i = 0; i < cur_frame_decisions_.size(); i++)
		{
			string log;
			days_remaining -= cur_frame_decisions_[i].action_->execute(cur_frame_decisions_[i].params_, this, -1, log);
			action_logs_.push_back(log);
		}

		if (days_remaining > 0)
		{
			for (int i = 0; i < free_time_decisions_.size(); i++)
			{
				string log;
				float days_to_do = free_time_decisions_[i].priority_weight_ * days_remaining;
				free_time_decisions_[i].action_->execute(free_time_decisions_[i].params_, this, days_to_do, log);
				action_logs_.push_back(log);
			}
		}

		cur_frame_decisions_.clear();
		free_time_decisions_.clear();

		knowledge_.tick();

		const float daily_pop_increase = 0.5 * 0.3 * 0.1 * 0.03;
		float percentage = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25f;
		pop_count_ += percentage * daily_pop_increase;

		for (int i = 0; i < needs_.size(); i++)
		{
			needs_[i]->apply(this);
		}
	}

	void Pop::decide()
	{
		//hunt if food supply is lacking
		for (int i = 0; i < needs_.size(); i++)
		{
			Need *cur_need = needs_[i];
			bool need_to_act = false;
			int needed_plain_item_index = -1;
			float needed_amount = -1;

			const map<int, float>& req = needs_[i]->get_item_requirements();
			for (auto it = req.begin(); it != req.end(); it++)
			{
				needed_amount = (it->second * pop_count_);
				if (get_plain_item_amount(it->first) < needed_amount)
				{
					need_to_act = true;
					needed_plain_item_index = it->first;
					break;

					//TODO_MURAT : multi item need per hunt
				}
			}

			if (need_to_act)
			{
				bool need_handled = false;
				int animal_to_hunt = -1;
				vector<int> animal_indeces_in_tile;
				g_world_instance_->get_tile(current_tile_)->get_animal_ids_in_tile(animal_indeces_in_tile);

				for (int i = 0; i < animal_indeces_in_tile.size(); i++)
				{
					Animal *animal_in_tile = (Animal *)g_world_instance_->get_object_by_index(animal_indeces_in_tile[i]);
					if (animal_in_tile->has_item_yield(needed_plain_item_index))
					{
						float base_yield = animal_in_tile->get_item_yield(needed_plain_item_index);
						int number_of_animals_needed = needed_amount / base_yield;

						cur_frame_decisions_.push_back(ActionDecision((Action*)g_world_instance_->get_object_by_name("Hunt"), { animal_indeces_in_tile[i],number_of_animals_needed }));
						need_handled = true;
					}
				}

				if (need_handled == false)
				{
					int next_tile = g_world_instance_->get_random_neighbour_tile(current_tile_);
					cur_frame_decisions_.push_back(ActionDecision((Action*)g_world_instance_->get_object_by_name("Travel"), { next_tile }));
				}
			}
		}

		//observe at remaining time
		{
			static const int number_of_things_to_observe = 3;
			vector<int> things_to_observe;
			g_world_instance_->get_tile(current_tile_)->get_things_in_tile(things_to_observe);
			get_plain_item_ids(things_to_observe);

			int count = things_to_observe.size();
			if (count > 0)
			{
				struct FreeActionDef
				{
					int thing_id;
					float weight;

					bool operator < (const FreeActionDef& str) const
					{
						return (weight > str.weight);
					}
				};
				vector<FreeActionDef> actions;
				float total_weight = 0;

				for (int i = 0; i < things_to_observe.size(); i++)
				{
					int thing_to_observe = things_to_observe[i];
					int knowledge_id = g_world_instance_->get_object_by_index(thing_to_observe)->get_associated_knowledge_id();

					float weight = get_knowledge().get_knowledge(knowledge_id);

					FreeActionDef new_def;
					new_def.thing_id = thing_to_observe;
					new_def.weight = weight > 0 ? weight : 0.25f;
					actions.push_back(new_def);
				}

				//sort them by weight
				std::sort(actions.begin(), actions.end());
				vector<FreeActionDef> used_actions;
				int random_index = g_world_instance_->get_random_value()* (actions.size() - 1);
				used_actions.push_back(actions[random_index]);
				actions.erase(actions.begin() + random_index);

				if (actions.size() > 0)
				{
					used_actions.push_back(actions[0]);
					actions.erase(actions.begin());
				}

				if (actions.size() > 0)
				{
					used_actions.push_back(actions[0]);
					actions.erase(actions.begin());
				}

				for (int i = 0; i < used_actions.size(); i++)
				{
					total_weight += used_actions[i].weight;
				}

				if (used_actions.size() < 3 || g_world_instance_->get_random_value() > 0.6)
				{
					FreeActionDef new_def;
					new_def.thing_id = g_world_instance_->get_tile(current_tile_)->get_id();
					new_def.weight = total_weight * 0.33;
					used_actions.push_back(new_def);

					total_weight += new_def.weight;
				}



				std::sort(used_actions.begin(), used_actions.end());

				for (int i = 0; i < used_actions.size(); i++)
				{
					if (used_actions[i].weight > 0)
					{
						int thing_id = used_actions[i].thing_id;
						float normalized_weight = total_weight > 0 ? used_actions[i].weight / total_weight : 1.0f;
						ZEPHYR_ASSERT(normalized_weight > 0.0f && normalized_weight < 1.0f);
						free_time_decisions_.push_back(ActionDecision((Action*)g_world_instance_->get_object_by_name("Observe"), { thing_id }, normalized_weight));
					}
				}
			}
		}
	}

	void Pop::give_plain_items(vector<pair<int, float>> &items)
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
				plain_items_owned_[items[i].first] = items[i].second;
			}
		}
	}

	void Pop::give_plain_item(int item_index, float amount)
	{
		auto it = plain_items_owned_.find(item_index);
		if (it != plain_items_owned_.end())
		{
			it->second += amount;
		}
		else
		{
			plain_items_owned_[item_index] = amount;
		}
	}

	void Pop::set_current_tile(int index)
	{
		if (current_tile_ != -1)
		{
			g_world_instance_->get_tile(current_tile_)->remove_living_pop(this->pop_id_);
		}
		current_tile_ = index;
		if (current_tile_ != -1)
		{
			g_world_instance_->get_tile(current_tile_)->add_living_pop(this->pop_id_);
		}
	}

	void Pop::add_knowledge(int knowledge_index, float amount)
	{
		knowledge_.add_knowledge(knowledge_index, amount);
	}

	int Pop::get_current_tile() const
	{
		return current_tile_;
	}

	const KnowledgeHistory & Pop::get_knowledge() const
	{
		return knowledge_;
	}

	float Pop::get_plain_item_amount(int index) const
	{
		auto it = plain_items_owned_.find(index);
		if (it != plain_items_owned_.end())
		{
			return it->second;
		}
		else
		{
			return 0.0f;
		}
	}

	float Pop::get_pop_count() const
	{
		return pop_count_;
	}

	bool Pop::can_inventory_saatisfy_need(Need * need) const
	{
		const map<int, float>& req = need->get_item_requirements();
		for (auto it = req.begin(); it != req.end(); it++)
		{
			if (get_plain_item_amount(it->first) < (it->second * pop_count_))
			{
				return false;
			}
		}

		return true;
	}

	void Pop::render_gui()
	{
		ImGui::Begin("Pop Data");
		ImGui::Text("Pop count %f", pop_count_);
		ImGui::Text("Tile Index %d", current_tile_);

		if (ImGui::TreeNode("Possessions"))
		{
			for (auto it = plain_items_owned_.begin(); it != plain_items_owned_.end(); it++)
			{
				ImGui::Text("Item (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Knowledge"))
		{
			knowledge_.render_gui();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Logs"))
		{
			for (int i = action_logs_.size() - 1; i >= 0; i--)
			{
				string log_tree_node = Utilities::formatted_string("Log%d", i);
				if (ImGui::TreeNode(log_tree_node.c_str()))
				{
					ImGui::Text(action_logs_[i].c_str());
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::End();
	}

	PlainItem::PlainItem(string name) : ObjectDefition( name, ObjectDefition::Type::plain_item )
	{
		knowledge_index_ = (new Knowledge(name + "_knowledge", ObjectDefition::Type::plain_item))->get_id();
	}

	inline ObjectDefition::ObjectDefition(string name, Type type) : type_(type) , name_(name)
	{
		index_ = g_world_instance_->get_index();
		g_world_instance_->register_object_definition(this);
	}

	int ObjectDefition::get_associated_knowledge_id() const
	{
		int knowledge_id = 0;
		switch (get_type())
		{
		case ObjectDefition::Type::animal:
		{
			const Animal *animal = (const Animal *)this;
			knowledge_id = animal->get_animal_knowledge_index();
			break;
		}
		case ObjectDefition::Type::plant:
		{
			const Plant *plant = (const Plant *)this;
			knowledge_id = plant->get_plant_knowledge_index();
			break;
		}
		case ObjectDefition::Type::plain_item:
		{
			const PlainItem *plain_item = (const PlainItem *)this;
			knowledge_id = plain_item->get_knowledge_index();
			break;
		}
		case ObjectDefition::Type::tile:
		{
			const Tile *tile_item = (const Tile *)this;
			knowledge_id = tile_item->get_tile_knowledge();
			break;
		}
		default:
			ZEPHYR_ASSERT(false);
		}

		return knowledge_id;
	}

	Knowledge::Knowledge(string name, ObjectDefition::Type knowledge_type) : ObjectDefition(name, ObjectDefition::Type::knowledge)
	{
		ZEPHYR_ASSERT(knowledge_type != ObjectDefition::Type::knowledge);
		knowledge_type_ = knowledge_type;
	}

	Need::Need(string name, initializer_list<pair<string, float>> daily_requirement, float priority) : ObjectDefition(name, ObjectDefition::Type::need)
	{
		prioirty_ = priority;
		for (auto it = daily_requirement.begin(); it != daily_requirement.end(); it++)
		{
			int plain_item_id = g_world_instance_->get_object_by_name(it->first)->get_id();
			plain_item_requirements_.insert(make_pair(plain_item_id, it->second));
		}
	}

	void Need::apply(Pop * pop)
	{
		for (auto it = plain_item_requirements_.begin(); it != plain_item_requirements_.end(); it++)
		{
			float amount_needed = pop->get_pop_count() * it->second;
			if (pop->get_plain_item_amount(it->first) > amount_needed)
			{
				pop->give_plain_item(it->first, -amount_needed);
			}
			else
			{
				//TODO_MURAT : penalty for not handling
			}
		}
	}

	Geography::Geography(const std::vector<VoronoiSolver::VoronoiSite>& voronoi_cells)
	{
		climates_.push_back(new Climate("Sea", {}, {}, -1, -0.25, D3DXVECTOR3(0, 0, 128), 0.0f));
		climates_.push_back(new Climate("Shore", {make_pair("sheep",50)}, {}, -0.25, 0, D3DXVECTOR3(0, 255, 255), 0));
		climates_.push_back(new Climate("Sand", { make_pair("sheep",70) }, {}, 0, 0.125, D3DXVECTOR3(240, 240, 64), 0));
		climates_.push_back(new Climate("Grass", { make_pair("sheep",100) }, {}, 0.125, 0.3750, D3DXVECTOR3(32, 160, 0), 0));
		climates_.push_back(new Climate("Forest", { make_pair("deer",80) , make_pair("sheep",30) }, {}, 0.375, 0.75, D3DXVECTOR3(13, 60, 0), 0.33));
		climates_.push_back(new Climate("Rocky", { make_pair("deer",50) }, {}, 0.75, 1, D3DXVECTOR3(128, 128, 128), 0.66));
		climates_.push_back(new Climate("Mountain", { make_pair("deer",10) }, {}, 1, 5, D3DXVECTOR3(255, 255, 255), 1));

		for (int i = 0; i < voronoi_cells.size(); i++)
		{
			float height = voronoi_cells[i].point.z;
			Climate *climate = nullptr;
			for (int j = 0; j < climates_.size(); j++)
			{
				if (climates_[j]->get_min() <= height && climates_[j]->get_max() >= height)
				{
					climate = climates_[j];
					break;
				}
			}

			tiles_.insert(make_pair(i, new Tile(D3DXVECTOR3(voronoi_cells[i].point.x, voronoi_cells[i].point.y, height), i, voronoi_cells[i].neighbours, climate)));
			number_of_tiles_++;
		}
	}

	Climate::Climate(string name, initializer_list<pair<string, int>> animal_pop, initializer_list<pair<string, int>> fauna,
		float min_height, float max_height, D3DXVECTOR3 rgb, float hunting_difficulty) : ObjectDefition(name, ObjectDefition::Type::climate)
	{
		for (auto it = animal_pop.begin(); it != animal_pop.end(); it++)
		{
			ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
			SAFEZEPHYR_ASSERT(obj && obj->get_type() == ObjectDefition::Type::animal)
			{
				animal_pop_.insert(make_pair(obj->get_id(), it->second));
			}
		}

		for (auto it = fauna.begin(); it != fauna.end(); it++)
		{
			ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
			SAFEZEPHYR_ASSERT(obj && obj->get_type() == ObjectDefition::Type::plant)
			{
				animal_pop_.insert(make_pair(obj->get_id(), it->second));
			}
		}

		min_height_ = min_height;
		max_height_ = max_height;
		middle_height_ = (max_height_ + min_height_) * 0.5f;
		rgb_ = rgb;
		hunt_difficulty_ = hunting_difficulty;
	}

}
