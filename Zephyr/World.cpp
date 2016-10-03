
#include "World.h"

using namespace Evolution;

namespace Evolution
{
	//--TODO_LIST
	// (DONE)execute functions should take argument for hours spent (-1 for enough)
	// (DONE)implement AI (decide function) , there should be primary and secondary jobs
	// (DONE)implement ticks , daily(food consumption) and weekly(health, pop and exp reduce)
	// (DONE)fill tiles with pop counts
	// animal reproduction
	// start ticking the first game
	// knowledge pheomena
	// gather action ?
	// usable item implementation and stone age..
	//-----------

	WorldInstance *g_world_instance_;

	WorldInstance::WorldInstance(int seed, const std::vector<VoronoiSolver::VoronoiSite> &voronoi_cells)
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
			needs.push_back(new Need("hunger", { make_pair("food", 0.03) } , 1.0f));
		}

		geography_ = new Geography(voronoi_cells);

		static const int number_of_pops = 5000;
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

			int base_pop_spawn_count = 20;
			float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;

			Pop *new_pop = new Pop(base_pop_spawn_count * percent);
			new_pop->set_current_tile(tile_to_spawn_index);
			new_pop->set_needs(needs);
			pops_.push_back(new_pop);
		}
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

	Tile::Tile(const D3DXVECTOR3 & center, int tile_index, vector<int> neighbours, Climate * climate)
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
	}

	void Tile::reduce_animal_pop(int animal_id, int number)
	{
		auto it = animal_pop_.find(animal_id);
		SAFE_ASSERT(it != animal_pop_.end())
		{
			it->second -= number;
			_ASSERT(it->second >= 0);
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
		SAFE_ASSERT(tile != nullptr)
		{
			vector<int> neighbours = tile->get_neighbours();
			int neighbour_count = neighbours.size();

			SAFE_ASSERT(neighbour_count > 0)
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
		_ASSERT(it == name_to_index_map_.end());

		name_to_index_map_[object->get_name()] = object->get_id();
		index_to_type_map_[object->get_id()] = object->get_type();
		index_to_object_map_[object->get_id()] = object;

		object_definitions_.push_back(object);
	}

	void WorldInstance::tick()
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

	float TravelAction::execute(std::vector<int> int_parameters, Pop * executer, float spent_hours)
	{
		int current_tile = executer->get_current_tile();
		int next_tile = int_parameters[0];

		float distance = g_world_instance_->get_distance_between_tiles(current_tile, next_tile);

		_ASSERT(distance > 0);

		//TODO_MURAT : side effects of travel, knowledge gain
		//TODO_MURAT : every travel finishes at one day ? 

		executer->set_current_tile(next_tile);

		return 0.0f;
	}

	float ObserveAction::execute(std::vector<int> int_parameters, Pop * executer, float spent_hours)
	{
		//TODO_MURAT : handle the effect of "amount of things observed"

		int observed_object_id = int_parameters[0];
		ObjectDefition *object = g_world_instance_->get_object_by_index(observed_object_id);
		int knowledge_id = -1;

		switch (object->get_type())
		{
		case ObjectDefition::Type::animal:
		{
			Animal *animal = (Animal *)object;
			knowledge_id = animal->get_animal_knowledge_index();
			break;
		}
		case ObjectDefition::Type::plant:
		{
			Plant *plant = (Plant *)object;
			knowledge_id = plant->get_plant_knowledge_index();
			break;
		}
		case ObjectDefition::Type::plain_item:
		{
			PlainItem *plain_item = (PlainItem *)object;
			knowledge_id = plain_item->get_knowledge_index();
			break;
		}
		default:
			_ASSERT(false);
		}

		const float max_exp_gained = 0.001f;
		float exp_gained = g_world_instance_->get_random_value() * max_exp_gained;

		executer->add_knowledge(knowledge_id, exp_gained);

		return 0.0f;
	}

	HuntAction::HuntAction() : Action("Hunt")
	{
		vector<int> animal_defs;
		g_world_instance_->get_indexes_of_type(ObjectDefition::Type::animal, animal_defs);

		for (int i = 0; i < animal_defs.size(); i++)
		{
			int animal_index = animal_defs[i];
			string name = g_world_instance_->get_object_by_index(animal_index)->get_name() + string("_hunting");
			int hunting_knowledge_index = (new Knowledge(name, ObjectDefition::Type::action))->get_id();
			animal_index_to_hunting_knowledge_index_.insert(make_pair(animal_index, hunting_knowledge_index));
		}
	}

	float HuntAction::execute(std::vector<int> int_parameters,Pop * executer, float spent_hours)
	{
		int hunted_animal_id = int_parameters[0];
		int desired_hunt_count = int_parameters[1];
		
		Tile *cur_tile = g_world_instance_->get_tile(executer->get_current_tile());
		int cur_tile_animal_pop = cur_tile->get_animal_population(hunted_animal_id);
		_ASSERT(cur_tile_animal_pop >= desired_hunt_count);

		const Animal* animal = (const Animal*)g_world_instance_->get_object_by_index(hunted_animal_id);

		//hunt efficency of pop
		float general_hunt_exp = executer->get_knowledge().get_knowledge( animal->get_animal_knowledge_index());
		float spcific_animal_hunt_exp = executer->get_knowledge().get_knowledge( animal_index_to_hunting_knowledge_index_[hunted_animal_id] );
		float cur_animal_hunt_difficulty = animal->get_hunt_difficulty();

		float total_hunting_exp_factor = general_hunt_exp + spcific_animal_hunt_exp;

		//TODO_MURAT : hunter amount ?
		float hours_to_hunt = spent_hours > 0.0f ? spent_hours : 16.0f;
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

			if (number_of_catches >= desired_hunt_count || time_spent > hours_to_hunt)
			{
				break;
			}
		}

		if (number_of_catches > 0)
		{
			vector<pair<int, float>> total_yield;
			animal->get_total_yield(number_of_catches, total_yield);
			executer->give_plain_items(total_yield);

			cur_tile->reduce_animal_pop(hunted_animal_id, number_of_catches);
		}

		executer->add_knowledge(animal->get_animal_knowledge_index(), total_exp_gained * 0.5f);
		executer->add_knowledge(animal_index_to_hunting_knowledge_index_[hunted_animal_id], total_exp_gained);

		return max(0, time_spent - hours_to_hunt);
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
			it->second = amount;
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
			it->second *= 0.95;
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

		hunt_difficulty = hunt_difficulty_;
		animal_knowledge_index_ = (new Knowledge(name + string("_animal"), ObjectDefition::Type::animal))->get_id();
	}

	Pop::Pop(int count)
	{
		pop_count_ = count;
	}

	void Pop::tick()
	{
		float hours_worked = 0.0f;
		for (int i = 0; i < cur_frame_decisions_.size(); i++)
		{
			cur_frame_decisions_[i].action_->execute(cur_frame_decisions_[i].params_, this, -1);
		}

		knowledge_.tick();

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
			vector<int> animal_indeces_in_tile;
			g_world_instance_->get_tile(current_tile_)->get_animal_ids_in_tile(animal_indeces_in_tile);

			int animal_count = animal_indeces_in_tile.size();
			if (animal_count > 0)
			{
				int animal_to_observe = animal_indeces_in_tile[g_world_instance_->get_random_value() * (animal_count - 1)];
				cur_frame_decisions_.push_back(ActionDecision((Action*)g_world_instance_->get_object_by_name("Observe"), { animal_to_observe }));
			}
		}
	}

	void Pop::give_plain_items(vector<pair<int, float>> items)
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

	void Pop::give_plain_item(int item_index, float amount)
	{
		auto it = plain_items_owned_.find(item_index);
		if (it != plain_items_owned_.end())
		{
			it->second += amount;
		}
		else
		{
			it->second = amount;
		}
	}

	void Pop::set_current_tile(int index)
	{
		current_tile_ = index;
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

	int Pop::get_pop_count() const
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

	PlainItem::PlainItem(string name) : ObjectDefition( name, ObjectDefition::Type::plain_item )
	{
		knowledge_index_ = (new Knowledge(name + "_knowledge", ObjectDefition::Type::plain_item))->get_id();
	}

	inline ObjectDefition::ObjectDefition(string name, Type type) : type_(type) , name_(name)
	{
		index_ = g_world_instance_->get_index();
		g_world_instance_->register_object_definition(this);
	}

	Knowledge::Knowledge(string name, ObjectDefition::Type knowledge_type) : ObjectDefition(name, ObjectDefition::Type::knowledge)
	{
		_ASSERT(knowledge_type != ObjectDefition::Type::knowledge);
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
		climates_.push_back(new Climate("Sea", {}, {}, -1, -0.25, D3DXVECTOR3(0, 0, 128)));
		climates_.push_back(new Climate("Shore", {make_pair("sheep",50)}, {}, -0.25, 0, D3DXVECTOR3(0, 255, 255)));
		climates_.push_back(new Climate("Sand", { make_pair("sheep",70) }, {}, 0, 0.125, D3DXVECTOR3(240, 240, 64)));
		climates_.push_back(new Climate("Grass", { make_pair("sheep",100) }, {}, 0.125, 0.3750, D3DXVECTOR3(32, 160, 0)));
		climates_.push_back(new Climate("Forest", { make_pair("deer",80) , make_pair("sheep",30) }, {}, 0.375, 0.75, D3DXVECTOR3(13, 60, 0)));
		climates_.push_back(new Climate("Rocky", { make_pair("deer",50) }, {}, 0.75, 1, D3DXVECTOR3(128, 128, 128)));
		climates_.push_back(new Climate("Snow", { make_pair("deer",10) }, {}, 1, 5, D3DXVECTOR3(255, 255, 255)));

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
		float min_height, float max_height, D3DXVECTOR3 rgb) : ObjectDefition(name, ObjectDefition::Type::climate)
	{
		for (auto it = animal_pop.begin(); it != animal_pop.end(); it++)
		{
			ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
			SAFE_ASSERT(obj && obj->get_type() == ObjectDefition::Type::animal)
			{
				animal_pop_.insert(make_pair(obj->get_id(), it->second));
			}
		}

		for (auto it = fauna.begin(); it != fauna.end(); it++)
		{
			ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
			SAFE_ASSERT(obj && obj->get_type() == ObjectDefition::Type::plant)
			{
				animal_pop_.insert(make_pair(obj->get_id(), it->second));
			}
		}

		min_height_ = min_height;
		max_height_ = max_height;
		middle_height_ = (max_height_ + min_height_) * 0.5f;
		rgb_ = rgb;
	}

}
