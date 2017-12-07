
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
	// (DONE)global variable for days per tick
	// (DONE)convert to weakly ticks and ai
	// (DONE)free time concept and weighted random for free time orderings and percentages
	// (DONE)gathering rocks, woods from trees
	// (DONE)knowledge discover concept , crafting items concept and crafting item act
	// (DONE)gather action ?
	// (DONE)free time actions rehaul -> do every action ?
	// crafting action , crafted item quality parameter
	// inventory class
	// crafting ai, first for needs than for best and most have items
	// item efficency for actions and item depletion when used
	// health and penalty for hunger
	// animal knowledge for yield of animal
	// special people with special knowledge gain bonuses
	// people vs pop knowledge transfer
	//-----------

	//WorldInstance *g_world_instance_;
	//
	//map<int, int> GatherAction::thing_index_to_gathering_index_;
	//map<int, int> HuntAction::animal_index_to_hunting_action_;
	//map<int, int> CraftingAction::item_index_to_crafting_action_;
	//
	//WorldInstance::WorldInstance(int seed, const std::vector<VoronoiSolver::VoronoiSite> &voronoi_cells) : days_per_tick_(7)
	//{
	//	memset(hof_filter_, 0, 512);
	//	g_world_instance_ = this;
	//
	//	srand(seed);
	//	last_index_ = -1;
	//	
	//	//plain items
	//	{
	//		new PlainItem("meat", {ObjectDefition::food});
	//		new PlainItem("berry", {ObjectDefition::food});
	//		new PlainItem("wood" );
	//		new PlainItem("stone", {ObjectDefition::gatherable});
	//		new PlainItem("wood" );
	//		new PlainItem("wool" );
	//		new PlainItem("hide" );
	//	}
	//
	//	//animals
	//	{
	//		new Animal("sheep", { make_pair("meat",0.5) , make_pair("wool",0.4), make_pair("hide",0.1) }, 0.3);
	//		new Animal("deer", { make_pair("meat",0.9) , make_pair("hide",0.3) }, 0.6);
	//	}
	//
	//	//plants
	//	{
	//		new Plant("tree", { make_pair("wood",15), make_pair("berry" , 3)}, 0.1);
	//	}
	//
	//	//crafted items
	//	{
	//		new CraftedItem("sculped_stone", { make_tuple("stone",1.8,0.1,0.5) }, {});
	//	}
	//
	//	//actions
	//	{
	//		TravelAction::initialize_action();
	//		ObserveAction::initialize_action();
	//		HuntAction::initialize_action();
	//		GatherAction::initialize_action();
	//		CraftingAction::initialize_action();
	//	}
	//
	//	vector<Need*> needs;
	//
	//	//need
	//	{
	//		needs.push_back(new Need("hunger", { make_pair(ObjectDefition::food, 0.03 * days_per_tick_) } , 1.0f));
	//	}
	//
	//	geography_ = new Geography(voronoi_cells);
	//
	//	static const int number_of_pops = 1000;
	//	int tile_count = geography_->get_number_of_tiles();
	//	for (int i = 0; i < number_of_pops; i++)
	//	{
	//		Tile *tile_to_spawn = nullptr;
	//		int tile_to_spawn_index = -1;
	//		while (true)
	//		{
	//			int index_cand = g_world_instance_->get_random_index(tile_count);
	//			Tile *tile_cand = geography_->get_tile(index_cand);
	//			if (tile_cand && tile_cand->get_climate()->get_name() != "Sea")
	//			{
	//				tile_to_spawn = tile_cand;
	//				tile_to_spawn_index = index_cand;
	//				break;
	//			}
	//		}
	//
	//		int base_pop_spawn_count = 25;
	//		float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;
	//
	//		Pop *new_pop = new Pop(base_pop_spawn_count * percent, pops_.size());
	//		new_pop->set_current_tile(tile_to_spawn_index);
	//		new_pop->set_needs(needs);
	//		new_pop->initialize_knowledge();
	//		pops_.push_back(new_pop);
	//	}
	//
	//	automatic_tick_ = false;
	//}
	//
	//float WorldInstance::get_random_value()
	//{
	//	return float(rand() % RAND_MAX) / (float)RAND_MAX;
	//}
	//
	//int WorldInstance::get_random_index(int count)
	//{
	//	return rand() % count;
	//}
	//
	//float WorldInstance::get_distance_between_tiles(int first_tile, int second_tile)
	//{
	//	D3DXVECTOR3 first_tile_position = geography_->get_tile(first_tile)->get_position();
	//	D3DXVECTOR3 second_tile_position = geography_->get_tile(second_tile)->get_position();
	//
	//	D3DXVECTOR3 diff = second_tile_position - first_tile_position;
	//
	//	return sqrt(D3DXVec3Dot(&diff, &diff));
	//}
	//
	//Tile::Tile(const D3DXVECTOR3 & center, int tile_index, vector<int> neighbours, Climate * climate) : ObjectDefition(Utilities::formatted_string("tile%d", tile_index), ObjectDefition::Type::tile)
	//{
	//	center_ = center;
	//	tile_index_ = tile_index;
	//	neighbours_ = neighbours;
	//	climate_ = climate;
	//
	//	const map<int, float>& animals = climate->get_animal_pop();
	//	for (auto it = animals.begin(); it != animals.end(); it++)
	//	{
	//		float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;
	//		animal_pop_[it->first] = percent * it->second;
	//	}
	//
	//	const map<int, float>& plants = climate->get_fauna();
	//	for (auto it = plants.begin(); it != plants.end(); it++)
	//	{
	//		float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;
	//		plant_pop_[it->first] = percent * it->second;
	//	}
	//
	//	const map<int, float>& plain_things = climate->get_plain_things_in_tile();
	//	for (auto it = plain_things.begin(); it != plain_things.end(); it++)
	//	{
	//		float percent = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25;
	//		plain_things_amount_[it->first] = percent * it->second;
	//	}
	//
	//	string knowledge_name = Utilities::formatted_string("tile%d_knowledge", tile_index);
	//	tile_knowledge_ = (new Knowledge(knowledge_name, ObjectDefition::Type::tile, tile_index))->get_id();
	//}
	//
	//void Tile::reduce_animal_pop(int animal_id, int number)
	//{
	//	auto it = animal_pop_.find(animal_id);
	//	ZEPHYR_SAFE_ASSERT(it != animal_pop_.end())
	//	{
	//		it->second -= number;
	//		ZEPHYR_ASSERT(it->second >= 0);
	//	}
	//}
	//
	//int Tile::get_animal_population(int animal_index) const
	//{
	//	auto it = animal_pop_.find(animal_index);
	//	if (it != animal_pop_.end())
	//	{
	//		return it->second;
	//	}
	//	return 0;
	//}
	//
	//void Tile::get_animal_ids_in_tile(vector<int>& animal_indeces) const
	//{
	//	for (auto it = animal_pop_.begin(); it != animal_pop_.end(); it++)
	//	{
	//		if (it->second > 0)
	//		{
	//			animal_indeces.push_back(it->first);
	//		}
	//	}
	//}
	//
	//void Tile::get_plant_ids_in_tile(vector<int>& plant_indeces) const
	//{
	//	for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
	//	{
	//		if (it->second > 0)
	//		{
	//			plant_indeces.push_back(it->first);
	//		}
	//	}
	//}
	//
	//void Tile::get_items_in_tile(vector<pair<int, float>>& items) const
	//{
	//	for (auto it = animal_pop_.begin(); it != animal_pop_.end(); it++)
	//	{
	//		items.push_back(make_pair(it->first, it->second));
	//	}
	//
	//	for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
	//	{
	//		items.push_back(make_pair(it->first, it->second));
	//	}
	//
	//	for (auto it = plain_things_amount_.begin(); it != plain_things_amount_.end(); it++)
	//	{
	//		items.push_back(make_pair(it->first, it->second));
	//	}
	//
	//}
	//
	//void Tile::get_things_in_tile(vector<int>& things_in_tile)
	//{
	//	get_animal_ids_in_tile(things_in_tile);
	//	get_plant_ids_in_tile(things_in_tile);
	//}
	//
	//void Tile::render_gui()
	//{
	//	ImGui::Begin("Tile Info");
	//	ImGui::Text("Climate : %s\n", climate_->get_name().c_str());
	//
	//	if (ImGui::TreeNode("Animals"))
	//	{
	//		for (auto it = animal_pop_.begin(); it != animal_pop_.end(); it++)
	//		{
	//			ImGui::Text("Animal (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
	//		}
	//
	//		ImGui::TreePop();
	//	}
	//
	//	if (ImGui::TreeNode("Plants"))
	//	{
	//		for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
	//		{
	//			ImGui::Text("Plant (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
	//		}
	//
	//		ImGui::TreePop();
	//	}
	//
	//
	//	if (ImGui::TreeNode("Plain Items"))
	//	{
	//		for (auto it = plain_things_amount_.begin(); it != plain_things_amount_.end(); it++)
	//		{
	//			ImGui::Text("Item (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
	//		}
	//
	//		ImGui::TreePop();
	//	}
	//
	//	if (ImGui::TreeNode("Pops"))
	//	{
	//		for (int i = 0; i < living_pops_.size(); i++)
	//		{
	//			ImGui::Text("Pop id-%d", living_pops_[i]);
	//		}
	//
	//		ImGui::TreePop();
	//	}
	//
	//	ImGui::End();
	//}
	//
	//Tile * WorldInstance::get_tile(int tile_index) const
	//{
	//	return geography_->get_tile(tile_index);
	//}
	//
	//int WorldInstance::get_index()
	//{
	//	last_index_++;
	//	return last_index_;
	//}
	//
	//void WorldInstance::get_indexes_of_type(ObjectDefition::Type type, vector<int> &indexes)
	//{
	//	for (int i = 0; i < object_definitions_.size(); i++)
	//	{
	//		if (object_definitions_[i]->get_type() == type)
	//		{
	//			indexes.push_back(object_definitions_[i]->get_id());
	//		}
	//	}
	//}
	//
	//ObjectDefition * WorldInstance::get_object_by_name(string name) const
	//{
	//	auto it = name_to_index_map_.find(name);
	//	if (it != name_to_index_map_.end())
	//	{
	//		int index = it->second;
	//		auto index_it = index_to_object_map_.find(index);
	//		if (index_it != index_to_object_map_.end())
	//		{
	//			return index_it->second;
	//		}
	//	}
	//
	//	return nullptr;
	//}
	//
	//int WorldInstance::get_random_neighbour_tile(int tile_index) const
	//{
	//	Tile *tile = geography_->get_tile(tile_index);
	//	ZEPHYR_SAFE_ASSERT(tile != nullptr)
	//	{
	//		vector<int> neighbours = tile->get_neighbours();
	//		int neighbour_count = neighbours.size();
	//
	//		ZEPHYR_SAFE_ASSERT(neighbour_count > 0)
	//		{
	//			int random_index = g_world_instance_->get_random_index(neighbour_count);
	//			return neighbours[random_index];
	//		}
	//	}
	//
	//	return -1;
	//}
	//
	//void WorldInstance::register_object_definition(ObjectDefition *object, const initializer_list<ObjectDefition::ObjectProperty> &properties)
	//{
	//	auto it = name_to_index_map_.find(object->get_name());
	//	ZEPHYR_ASSERT(it == name_to_index_map_.end());
	//
	//	name_to_index_map_[object->get_name()] = object->get_id();
	//	index_to_type_map_[object->get_id()] = object->get_type();
	//	index_to_object_map_[object->get_id()] = object;
	//
	//	object_definitions_.push_back(object);
	//
	//	for (auto it = properties.begin(); it != properties.end(); it++)
	//	{
	//		items_with_properties_[*it].push_back(object->get_id());
	//	}
	//}
	//
	//void WorldInstance::tick()
	//{
	//	ImGui::Begin("World Simulation",nullptr);
	//
	//	ImGui::Columns(2);
	//
	//	ImGui::Checkbox("Automatic Tick", &automatic_tick_);
	//
	//	static int tick_count = 1;
	//	ImGui::InputInt("Tick Count", &tick_count);
	//	ImGui::SameLine();
	//	if (ImGui::Button("Single Tick"))
	//	{
	//		for (int i = 0; i < tick_count; i++)
	//		{
	//			tick_aux();
	//		}
	//	}
	//
	//	static int selected_pop_index = 0;
	//	ImGui::InputInt("Pop index", &selected_pop_index);
	//	
	//	selected_pop_index = max(selected_pop_index, 0);
	//	selected_pop_index = min(selected_pop_index, pops_.size() - 1);
	//
	//	static int selected_tile_index = 0;
	//	ImGui::InputInt("Tile Index", &selected_tile_index);
	//
	//	ImGui::NextColumn();
	//
	//	
	//	ImGui::InputText("Filter", hof_filter_, 512);
	//	{
	//		int number_of_values = 0;
	//		string hall_of_fame_filter(hof_filter_);
	//
	//		for (auto it = hall_of_fame_of_pops_knowledge_.begin(); it != hall_of_fame_of_pops_knowledge_.end(); it++)
	//		{
	//			ObjectDefition *cur_object = it->first;
	//			if (hall_of_fame_filter.length() == 0 || (cur_object->get_name().find(hall_of_fame_filter) != std::string::npos))
	//			{
	//				ImGui::Text("%s -> %d (%f)", cur_object->get_name().c_str(), it->second.first, it->second.second);
	//				number_of_values++;
	//			}
	//
	//			if (number_of_values == 10)
	//			{
	//				break;
	//			}
	//		}
	//	}
	//
	//	ImGui::End();
	//
	//	Tile *cur_tile = geography_->get_tile(selected_tile_index);
	//	if (cur_tile)
	//	{
	//		cur_tile->render_gui();
	//	}
	//
	//	pops_[selected_pop_index]->render_gui();
	//
	//	if (automatic_tick_)
	//	{
	//		tick_aux();
	//	}
	//}
	//
	//void WorldInstance::get_items_with_property(ObjectDefition::ObjectProperty key, vector<int>& items) const
	//{
	//	auto it = items_with_properties_.find(key);
	//	if (it != items_with_properties_.end())
	//	{
	//		items.insert(items.begin(), it->second.begin(), it->second.end());;
	//	}
	//}
	//
	//void WorldInstance::toggle_tile_render()
	//{
	//	tile_render_on_ = !tile_render_on_;
	//}
	//
	//void WorldInstance::tick_aux()
	//{
	//	for (int i = 0; i < pops_.size(); i++)
	//	{
	//		pops_[i]->decide();
	//	}
	//
	//	for (int i = 0; i < pops_.size(); i++)
	//	{
	//		pops_[i]->tick();
	//	}
	//}
	//
	//
	//void WorldInstance::register_new_knowledge(int pop_id, int knowledge_id, float exp)
	//{
	//	ObjectDefition *obj = g_world_instance_->get_object_by_index(knowledge_id);
	//	auto it = hall_of_fame_of_pops_knowledge_.find(obj);
	//	if (it != hall_of_fame_of_pops_knowledge_.end())
	//	{
	//		if (it->second.second < exp)
	//		{
	//			it->second = make_pair(pop_id, exp);
	//		}
	//	}
	//	else
	//	{
	//		hall_of_fame_of_pops_knowledge_.insert(make_pair(obj,make_pair(pop_id, exp)));
	//	}
	//}
	//
	//float Animal::get_item_yield(int index) const
	//{
	//	auto it = plain_item_yield_.find(index);
	//	if (it != plain_item_yield_.end())
	//	{
	//		return it->second;
	//	}
	//
	//	return 0.0f;
	//}
	//
	//float Animal::get_hunt_difficulty() const
	//{
	//	return hunt_difficulty_;
	//}
	//
	//void Animal::get_total_yield(float number_of_catches, vector<pair<int, float>>& total_yield) const
	//{
	//	for (auto it = plain_item_yield_.begin(); it != plain_item_yield_.end(); it++)
	//	{
	//		total_yield.push_back(make_pair(it->first, it->second * number_of_catches));
	//	}
	//
	//}
	//
	//float Animal::get_item_yield_with_property(ObjectDefition::ObjectProperty prop) const
	//{
	//	float amount = 0;
	//	for (auto it = plain_item_yield_.begin(); it != plain_item_yield_.end(); it++)
	//	{
	//		if (g_world_instance_->get_object_by_index(it->first)->has_property(prop))
	//		{
	//			amount += it->second;
	//		}
	//	}
	//
	//	return amount;
	//}
	//
	//void Animal::get_yield_ids(vector<int>& total_yield) const
	//{
	//	for (auto it = plain_item_yield_.begin(); it != plain_item_yield_.end(); it++)
	//	{
	//		total_yield.push_back(it->first);
	//	}
	//}
	//
	//Action::Action(string name) : ObjectDefition(name, ObjectDefition::Type::action)
	//{
	//}
	//
	//bool Action::has_yield_with_property_type(ObjectDefition::ObjectProperty obj_prop) const
	//{
	//	vector<int> yield_ids;
	//	get_yield_items(yield_ids);
	//
	//	for (int i = 0; i < yield_ids.size(); i++)
	//	{
	//		if (g_world_instance_->get_object_by_index(yield_ids[i])->has_property(obj_prop))
	//		{
	//			return true;
	//		}
	//	}
	//
	//	return false;
	//}
	//
	//float Action::get_yield_per_hour_of_property_type(ObjectDefition::ObjectProperty obj_prop, const KnowledgeHistory & knwoledge, 
	//	int tile_index, const map<int, float> &inventory) const
	//{
	//	float total_yield = 0;
	//	vector<pair<int, float>> yield_per_hour;
	//	get_yield_items_per_hour(yield_per_hour, knwoledge, tile_index, inventory);
	//
	//	for (int i = 0; i < yield_per_hour.size(); i++)
	//	{
	//		if (g_world_instance_->get_object_by_index(yield_per_hour[i].first)->has_property(obj_prop))
	//		{
	//			total_yield += yield_per_hour[i].second;
	//		}
	//	}
	//
	//	return total_yield;
	//}
	//
	//void Action::add_efficency_item(string name, float factor, float cease_factor_per_day)
	//{
	//	int item_id = g_world_instance_->get_object_by_name(name)->get_id();
	//	ZEPHYR_ASSERT(efficency_items_.find() == efficency_items_.end());
	//	ZEPHYR_ASSERT(factor > 0.0f && factor <= 1.0f);
	//	ZEPHYR_ASSERT(cease_factor_per_day >= 0.0f && cease_factor_per_day <= 1.0f);
	//
	//	efficency_items_[item_id] = make_pair(factor, cease_factor_per_day);
	//}
	//
	//float Action::get_item_efficency_from_inventory(const map<int, float>& inventory) const
	//{
	//	float total_efficency = 0.0f;
	//
	//	for (auto it = efficency_items_.begin(); it != efficency_items_.end(); it++)
	//	{
	//		if (inventory.find(it->first) != inventory.end())
	//		{
	//			total_efficency += get<0>(it->second);
	//		}
	//	}
	//
	//	return total_efficency;
	//}
	//
	//float TravelAction::execute(std::vector<float> int_parameters, Pop * executer, float spent_hours, string &log)
	//{
	//	int current_tile = executer->get_current_tile();
	//	int next_tile = int_parameters[0];
	//
	//	float distance = g_world_instance_->get_distance_between_tiles(current_tile, next_tile);
	//
	//	ZEPHYR_ASSERT(distance > 0);
	//
	//	//TODO_MURAT : side effects of travel, knowledge gain
	//	//TODO_MURAT : every travel finishes at one day ? 
	//
	//	executer->set_current_tile(next_tile);
	//
	//	log += Utilities::formatted_string("Moved from %d to %d\n", current_tile, next_tile);
	//
	//	return 1.0f;
	//}
	//
	//void TravelAction::initialize_action()
	//{
	//	new TravelAction();
	//}
	//
	//float ObserveAction::execute(std::vector<float> int_parameters, Pop * executer, float spent_days, string &log)
	//{
	//	int thing_id = int_parameters[0];
	//	ObjectDefition *thing_to_observe = g_world_instance_->get_object_by_index(thing_id);
	//	int knowledge_id = thing_to_observe->get_associated_knowledge_id();
	//
	//	Knowledge *knowledge = (Knowledge *)g_world_instance_->get_object_by_index(knowledge_id);
	//	ZEPHYR_ASSERT(knowledge->get_type() == ObjectDefition::Type::knowledge);
	//
	//	static const float days_spent_to_maximize_skill = 12 * 30;
	//	static const float exp_gained_per_day = 1.0f / days_spent_to_maximize_skill;
	//
	//	float exp_gained = (0.5 + g_world_instance_->get_random_value()) * exp_gained_per_day * spent_days;
	//
	//	log += Utilities::formatted_string("Observe action with days spent (%f).\n", spent_days);
	//
	//	if (thing_to_observe->get_type() == ObjectDefition::Type::tile)
	//	{
	//		Tile *cur_tile = (Tile *)thing_to_observe;
	//		vector<int> things_in_tile;
	//		cur_tile->get_things_in_tile(things_in_tile);
	//
	//		if (things_in_tile.size() > 0)
	//		{
	//			int index = g_world_instance_->get_random_index(things_in_tile.size());
	//			int extra_knowledge_id = g_world_instance_->get_object_by_index(things_in_tile[index])->get_associated_knowledge_id();
	//			float extra_exp_gained = (0.5 + g_world_instance_->get_random_value()) * exp_gained_per_day * spent_days;
	//		
	//			executer->add_knowledge(extra_knowledge_id, extra_exp_gained);
	//			log += Utilities::formatted_string("Observed (%s) , gained (%f) exp.\n", g_world_instance_->get_object_by_index(extra_knowledge_id)->get_name().c_str(), extra_exp_gained);
	//		}
	//
	//		exp_gained *= 0.75;
	//	}
	//
	//	executer->add_knowledge(knowledge_id, exp_gained);
	//
	//	log += Utilities::formatted_string("Observed (%s) , gained (%f) exp.\n", g_world_instance_->get_object_by_index(knowledge_id)->get_name().c_str(), exp_gained);
	//
	//	return spent_days;
	//}
	//
	//void ObserveAction::initialize_action()
	//{
	//	new ObserveAction();
	//}
	//
	//HuntAction::HuntAction(int hunted_id) : Action(g_world_instance_->get_object_by_index(hunted_id)->get_name() + string("_hunting_action"))
	//{
	//	hunted_id_ = hunted_id;
	//
	//	if (g_world_instance_->get_object_by_name("hunting_action_knowledge") == nullptr)
	//	{
	//		new Knowledge("hunting_action_knowledge", ObjectDefition::Type::action, -1, {});
	//	}
	//
	//	global_hunting_knowledge_id_ = g_world_instance_->get_object_by_name("hunting_action_knowledge")->get_id();
	//	string name = g_world_instance_->get_object_by_index(hunted_id)->get_name();
	//	hunting_knowledge_id_ = (new Knowledge(name + "_hunting_knowledge", ObjectDefition::Type::action, get_id()))->get_id();
	//
	//	add_efficency_item("sculped_stone", 0.1f, 0.02f);
	//}
	//
	//float HuntAction::execute(std::vector<float> float_parameters,Pop * executer, float spent_hours, string &log)
	//{
	//	const Animal* animal = (const Animal*)g_world_instance_->get_object_by_index(hunted_id_);
	//	
	//	float desired_food_amount = float_parameters[0];
	//	float animal_yield = animal->get_item_yield_with_property(ObjectDefition::food);
	//
	//	if (animal_yield <= 0)
	//	{
	//		int a = 5;
	//	}
	//
	//	int desired_hunt_count = ceil(desired_food_amount / animal_yield);
	//
	//	Tile *cur_tile = g_world_instance_->get_tile(executer->get_current_tile());
	//	int cur_tile_animal_pop = cur_tile->get_animal_population(hunted_id_);
	//	ZEPHYR_ASSERT(cur_tile_animal_pop >= desired_hunt_count);
	//
	//	float random_value = g_world_instance_->get_random_value();
	//
	//	float hunt_time = get_hunt_time(executer->get_knowledge(), executer->get_current_tile(), {}, random_value);
	//
	//	log += Utilities::formatted_string("Hunting efficency random value (%f).\n", random_value);
	//	//log += Utilities::formatted_string("Hunt total_efficency (%f), time multiplier (%f) , hunt time (%f).\n", total_efficency, 1.0f / time_factor, hunt_time);
	//
	//	float total_days_spent = min((desired_hunt_count * hunt_time), g_world_instance_->get_days_per_tick());
	//	int number_of_catches = total_days_spent / hunt_time;
	//
	//	log += Utilities::formatted_string("Spent hours (%f), wanted (%d), hunted #(%d) of (%s).\n", total_days_spent, desired_hunt_count, number_of_catches, animal->get_name().c_str());
	//
	//	if (number_of_catches > 0 && cur_tile_animal_pop >= number_of_catches)
	//	{
	//		vector<pair<int, float>> total_yield;
	//		animal->get_total_yield(number_of_catches, total_yield);
	//		executer->give_plain_items(total_yield);
	//
	//		for (auto it = total_yield.begin(); it != total_yield.end(); it++)
	//		{
	//			log += Utilities::formatted_string("Gained item(%s) of amount (%f).\n", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
	//		}
	//
	//		cur_tile->reduce_animal_pop(hunted_id_, number_of_catches);
	//	}
	//
	//	static const float days_spent_to_maximize_skill = 6 * 30;
	//	static const float exp_gained_per_day = 1.0f / days_spent_to_maximize_skill;
	//
	//	executer->add_knowledge(animal->get_animal_knowledge_index(), exp_gained_per_day * 0.25f * total_days_spent);
	//	executer->add_knowledge(global_hunting_knowledge_id_, exp_gained_per_day * 0.5f * total_days_spent);
	//	executer->add_knowledge(hunting_knowledge_id_, exp_gained_per_day * total_days_spent);
	//
	//	//log += Utilities::formatted_string("Gained exp(%s) of amount (%f).\n", g_world_instance_->get_object_by_index(animal->get_animal_knowledge_index())->get_name().c_str(), total_exp_gained * 0.5f);
	//	//log += Utilities::formatted_string("Gained exp(%s) of amount (%f).\n", g_world_instance_->get_object_by_index(animal_index_to_hunting_knowledge_index_[hunted_animal_id])->get_name().c_str(), total_exp_gained);
	//
	//	return total_days_spent;
	//}
	//
	//void HuntAction::get_yield_items(std::vector<int> &action_yield) const
	//{
	//	Animal *hunted_animal = (Animal *)g_world_instance_->get_object_by_index(hunted_id_);
	//	hunted_animal->get_yield_ids(action_yield);
	//}
	//
	//void HuntAction::get_yield_items_per_hour(std::vector<pair<int, float>>& actions_yield, const KnowledgeHistory & knwoledge, int tile_index, const map<int, float>& inventory) const
	//{
	//	Animal* animal = (Animal*)g_world_instance_->get_object_by_index(hunted_id_);
	//	float average_hunt_time = get_hunt_time(knwoledge, tile_index, inventory, 0.5f);
	//	animal->get_total_yield(1.0f / average_hunt_time, actions_yield);
	//}
	//
	//void HuntAction::initialize_action()
	//{
	//	vector<int> huntable_things;
	//	g_world_instance_->get_items_with_property(ObjectDefition::huntable, huntable_things);
	//
	//	for (int i = 0; i < huntable_things.size(); i++)
	//	{
	//		int hunted_id = huntable_things[i];
	//		int action_id = (new HuntAction(hunted_id))->get_id();
	//		animal_index_to_hunting_action_.insert(make_pair(hunted_id, action_id));
	//	}
	//}
	//
	//HuntAction * HuntAction::get_action_with_hunted_id(int hunted_id)
	//{
	//	auto it = animal_index_to_hunting_action_.begin();
	//	if (it != animal_index_to_hunting_action_.end())
	//	{
	//		return (HuntAction*)g_world_instance_->get_object_by_index(it->second);
	//	}
	//
	//	return nullptr;
	//}
	//
	//float HuntAction::get_hunt_time(const KnowledgeHistory & knwoledge, int tile_index, const map<int, float>& inventory, float random_value) const
	//{
	//	Animal* animal = (Animal*)g_world_instance_->get_object_by_index(hunted_id_);
	//
	//	float total_efficency = get_action_efficency(knwoledge, tile_index, inventory);
	//
	//	const float base_hunt_time_per_animal = 0.125f;
	//	float cur_animal_hunt_difficulty = animal->get_hunt_difficulty();
	//	float time_factor = total_efficency * (1.0f - cur_animal_hunt_difficulty) * (random_value * 2.0f);
	//	float hunt_time = base_hunt_time_per_animal / time_factor;
	//
	//	return hunt_time;
	//}
	//
	//float HuntAction::get_action_efficency(const KnowledgeHistory & knowledge, int tile_index, const map<int, float> &inventory) const
	//{
	//	//calculate hunt efficency
	//	//base -> 0.3
	//	//hunting exp -> 0.25 at 1.0
	//	//animal information -> 0.25 at 1.0
	//	//animal hunt exp -> 0.3 at 1.0
	//	//weapon -> 0.4
	//	//geographic -> 0.5
	//
	//	Tile *cur_tile = g_world_instance_->get_tile(tile_index);
	//	Animal* animal = (Animal*)g_world_instance_->get_object_by_index(hunted_id_);
	//
	//	float base_efficency = 0.3f;
	//	float hunting_exp = knowledge.get_knowledge(global_hunting_knowledge_id_) * 0.25;
	//	float animal_exp = knowledge.get_knowledge(animal->get_animal_knowledge_index()) * 0.25;
	//	float animal_hunt_exp = knowledge.get_knowledge(hunting_knowledge_id_) * 0.3;
	//	float weapon_factor = 0 * 0.4f;
	//	float geographic_factor = (1.0f - cur_tile->get_climate()->get_hunting_difficulty()) * 0.5;
	//
	//	return base_efficency + hunting_exp + animal_exp + animal_hunt_exp + weapon_factor + geographic_factor;
	//}
	//
	//GatherAction::GatherAction(int gathered_index) : Action(g_world_instance_->get_object_by_index(gathered_index)->get_name() + string("_gathering_action"))
	//{
	//	gathered_id_ = gathered_index;
	//
	//	if (g_world_instance_->get_object_by_name("gathering_action_knowledge") == nullptr)
	//	{
	//		new Knowledge("gathering_action_knowledge", ObjectDefition::Type::action, -1);
	//	}
	//
	//	global_gathering_knowledge_id_ = g_world_instance_->get_object_by_name("gathering_action_knowledge")->get_id();
	//	string name = g_world_instance_->get_object_by_index(gathered_index)->get_name();
	//	gathering_knowledge_id_ = (new Knowledge(name + "_gathering_knowledge", ObjectDefition::Type::action, get_id()))->get_id();
	//}
	//
	//void GatherAction::get_yield_items_per_hour(std::vector<pair<int, float>>& actions_yield, const KnowledgeHistory & knwoledge, int tile_index, const map<int, float>& inventory) const
	//{
	//	ObjectDefition *gathered_object = g_world_instance_->get_object_by_index(gathered_id_);
	//
	//	float random_value = g_world_instance_->get_random_value();
	//	float total_efficency = get_action_efficency(knwoledge, tile_index, inventory);
	//
	//	const float base_quantity_gathered_per_day = 0.6f;
	//	float gathered_per_day = base_quantity_gathered_per_day * total_efficency * (0.8 + random_value * 0.4);
	//
	//	bool gathered_thing_is_composite = gathered_object->get_type() != ObjectDefition::Type::plain_item;
	//	if (gathered_thing_is_composite)
	//	{
	//		Plant *plant = (Plant *)gathered_object;
	//		plant->get_yield_ids(actions_yield);
	//
	//		for (int i = 0; i < actions_yield.size(); i++)
	//		{
	//			actions_yield[i].second *= gathered_per_day;
	//		}
	//	}
	//	else
	//	{
	//		actions_yield.push_back(make_pair(gathered_id_, gathered_per_day));
	//	}
	//}
	//
	//float GatherAction::execute(std::vector<float> float_parameters, Pop * executer, float spent_days, string & log)
	//{
	//	ObjectDefition *gathered_object = g_world_instance_->get_object_by_index(gathered_id_);
	//	Tile *cur_tile = g_world_instance_->get_tile(executer->get_current_tile());
	//
	//	vector<pair<int,float>> gathered_stuff;
	//
	//	bool gathered_thing_is_composite = gathered_object->get_type() != ObjectDefition::Type::plain_item;
	//	if (gathered_thing_is_composite)
	//	{
	//		Plant *plant = (Plant *)gathered_object;
	//		plant->get_yield_ids(gathered_stuff);
	//	}
	//	else
	//	{
	//		gathered_stuff.push_back(make_pair(gathered_id_, 1.0f));
	//	}
	//
	//	if (spent_days == -1)	//do this until req is done
	//	{
	//		float amount_to_gather = float_parameters[0];
	//		float base_yield_for_need = 0.0f;
	//		for (int i = 0; i < gathered_stuff.size(); i++)
	//		{
	//			if (g_world_instance_->get_object_by_index(gathered_stuff[i].first)->has_property(ObjectDefition::food))
	//			{
	//				base_yield_for_need += gathered_stuff[i].second;
	//			}
	//		}
	//
	//		if (base_yield_for_need <= 0)
	//		{
	//			int a = 5;
	//		}
	//
	//		spent_days = amount_to_gather / base_yield_for_need;
	//	}
	//
	//	int thing_knowledge_index = g_world_instance_->get_object_by_index(gathered_id_)->get_associated_knowledge_id();
	//
	//	float random_value = g_world_instance_->get_random_value();
	//	float total_efficency = get_action_efficency(executer->get_knowledge(), executer->get_current_tile(), map<int, float>());
	//
	//	const float base_quantity_gathered_per_day = 0.6f;
	//	float gathered_per_day = base_quantity_gathered_per_day * total_efficency * (0.8 + random_value * 0.4);
	//
	//	float gathered_item = gathered_per_day * spent_days;
	//
	//	for (int i = 0; i < gathered_stuff.size(); i++)
	//	{
	//		executer->give_plain_item(gathered_stuff[i].first, gathered_item * gathered_stuff[i].second);
	//	}
	//
	//	const float days_to_master_gathering = 5 * 30;
	//	const float exp_gained_per_day = 1.0f / days_to_master_gathering;
	//	float learning_efficency = 0.5f + g_world_instance_->get_random_value();
	//	float gather_exp_gained = learning_efficency * exp_gained_per_day * spent_days;
	//
	//	if (gathered_thing_is_composite)
	//	{
	//		gather_exp_gained *= 0.8f;
	//	}
	//
	//	float item_exp_gained = learning_efficency * exp_gained_per_day * spent_days * 0.3f;
	//
	//	executer->add_knowledge(gathering_knowledge_id_, gather_exp_gained);
	//	executer->add_knowledge(thing_knowledge_index, item_exp_gained);
	//
	//	if (gathered_thing_is_composite)
	//	{
	//		for (int i = 0; i < gathered_stuff.size(); i++)
	//		{
	//			int composite_knowledge = g_world_instance_->get_object_by_index(gathered_stuff[i].first)->get_associated_knowledge_id();
	//			executer->add_knowledge(composite_knowledge, item_exp_gained * 0.4);
	//		}
	//	}
	//
	//	cur_tile->modify_thing_count(gathered_id_, -gathered_item);
	//
	//	log += Utilities::formatted_string("Gathering efficency random value (%f), gathering efficency (%f).\n", random_value, total_efficency);
	//	log += Utilities::formatted_string("Gathering (%f) of item (%s) .\n", total_efficency, gathered_object->get_name().c_str());
	//	log += Utilities::formatted_string("Learning efficency (%f) , gather exp(%f), item exp(%f).\n", learning_efficency, gather_exp_gained, item_exp_gained);
	//
	//	return spent_days;
	//}
	//
	//void GatherAction::get_yield_items(std::vector<int> &action_yield) const
	//{
	//	ObjectDefition *obj = g_world_instance_->get_object_by_index(gathered_id_);
	//	if (obj->get_type() == ObjectDefition::Type::plain_item)
	//	{
	//		action_yield.push_back(gathered_id_);
	//	}
	//	else if (obj->get_type() == ObjectDefition::Type::plant)
	//	{
	//		Plant *plant = (Plant *)obj;
	//
	//		vector<pair<int, float>> indices_with_amount;
	//		plant->get_yield_ids(indices_with_amount);
	//
	//		for (int i = 0; i < indices_with_amount.size(); i++)
	//		{
	//			action_yield.push_back(indices_with_amount[i].first);
	//		}
	//	}
	//}
	//
	//void GatherAction::initialize_action()
	//{
	//	vector<int> gatherable_things;
	//	g_world_instance_->get_items_with_property(ObjectDefition::gatherable, gatherable_things);
	//
	//	for (int i = 0; i < gatherable_things.size(); i++)
	//	{
	//		int gathered_id = gatherable_things[i];
	//		int action_id = (new GatherAction(gathered_id))->get_id();
	//		thing_index_to_gathering_index_.insert(make_pair(gathered_id, action_id));
	//	}
	//}
	//
	//float GatherAction::get_action_efficency(const KnowledgeHistory & knwoledge, int tile_index, const map<int, float>& inventory) const
	//{
	//	int thing_knowledge_index = g_world_instance_->get_object_by_index(gathered_id_)->get_associated_knowledge_id();
	//	
	//	float base_efficency = 0.5f;
	//	float gathering_knowledge_efficency = 0.6 * knwoledge.get_knowledge(gathering_knowledge_id_);
	//	float thing_knowledge = 0.4 * knwoledge.get_knowledge(thing_knowledge_index);
	//	return base_efficency + gathering_knowledge_efficency;
	//}
	//
	//float GatherAction::get_gather_time(const KnowledgeHistory & knwoledge, int tile_index, const map<int, float>& inventory, float random_value) const
	//{
	//	return 0.0f;
	//}
	//
	//GatherAction * GatherAction::get_action_with_gathered_id(int hunted_id)
	//{
	//	auto it = thing_index_to_gathering_index_.find(hunted_id);
	//
	//	if (it != thing_index_to_gathering_index_.end())
	//	{
	//		return (GatherAction*)g_world_instance_->get_object_by_index(it->second);
	//	}
	//
	//	return nullptr;
	//}
	//
	//KnowledgeHistory::KnowledgeHistory(int pop_id) : pop_index_(pop_id)
	//{
	//
	//}
	//
	//void KnowledgeHistory::add_knowledge(int index, float amount)
	//{
	//	if (amount > 1 || amount < 0)
	//	{
	//		int a = 5;
	//	}
	//
	//	auto it = knowledge_set_.find(index);
	//	if (it != knowledge_set_.end())
	//	{
	//		it->second += amount;
	//		g_world_instance_->register_new_knowledge(pop_index_, index, it->second);
	//	}
	//	else
	//	{
	//		knowledge_set_[index] = amount;
	//		g_world_instance_->register_new_knowledge(pop_index_, index, amount);
	//	}
	//
	//	Knowledge *cur_knowledge = (Knowledge *)g_world_instance_->get_object_by_index(index);
	//	cur_knowledge->trigger_discovery(*this);
	//
	//}
	//
	//float KnowledgeHistory::get_knowledge(int index) const
	//{
	//	auto it = knowledge_set_.find(index);
	//	if (it != knowledge_set_.end())
	//	{
	//		return it->second;
	//	}
	//
	//	return 0.0f;
	//}
	//
	//void KnowledgeHistory::tick()
	//{
	//	for (auto it = knowledge_set_.begin(); it != knowledge_set_.end(); it++)
	//	{
	//		it->second *= 0.98;
	//	}
	//}
	//
	//void KnowledgeHistory::render_gui()
	//{
	//	for (auto it = knowledge_set_.begin(); it != knowledge_set_.end(); it++)
	//	{
	//		if (it->second > 0)
	//		{
	//			ImGui::Text("Knowledge (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
	//		}
	//	}
	//}
	//
	//void KnowledgeHistory::get_knowledge_ids_with_type(ObjectDefition::Type type, vector<pair<int,float>>& ids) const
	//{
	//	for (auto it = knowledge_set_.begin(); it != knowledge_set_.end(); it++)
	//	{
	//		Knowledge *knowledge = (Knowledge *)g_world_instance_->get_object_by_index(it->first);
	//		if (knowledge->get_knowledge_type() == type && knowledge->get_associated_item_id() != -1)
	//		{
	//			ids.push_back(make_pair(it->first, it->second));
	//		}
	//	}
	//
	//	sort(ids.begin(), ids.end(),
	//		[](const pair<int, float> & a, const pair<int, float> & b)
	//	{
	//		return a.second > b.second;
	//	});
	//}
	//
	//Plant::Plant(string name, initializer_list<pair<string, float>> plain_item_yield, float gather_difficulty) : ObjectDefition(name, ObjectDefition::Type::plant, {ObjectDefition::gatherable})
	//{
	//	for (auto it = plain_item_yield.begin(); it != plain_item_yield.end(); it++) 
	//	{
	//		ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
	//		ZEPHYR_SAFE_ASSERT(obj && obj->get_type() == ObjectDefition::Type::plain_item)
	//		{
	//			int plain_item_index = obj->get_id();
	//			plain_item_yield_[plain_item_index] = it->second;
	//			((PlainItem*)obj)->add_container_id(get_id());
	//		}
	//	}
	//
	//	gather_difficulty_ = gather_difficulty;
	//
	//	plant_knowledge_index_ = (new Knowledge(name + string("_plant_knowledge"), ObjectDefition::Type::plant, get_id()))->get_id();
	//}
	//
	//void Plant::get_yield_ids(vector<pair<int, float>>& indices) const
	//{
	//	for (auto it = plain_item_yield_.begin(); it != plain_item_yield_.end(); it++)
	//	{
	//		indices.push_back(make_pair(it->first, it->second));
	//	}
	//}
	//
	//Animal::Animal(string name, initializer_list<pair<string, float>> plain_item_yield, float hunt_difficulty) : ObjectDefition(name, ObjectDefition::Type::animal, {ObjectDefition::huntable})
	//{
	//	for (auto it = plain_item_yield.begin(); it != plain_item_yield.end(); it++)
	//	{
	//		ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
	//		ZEPHYR_SAFE_ASSERT(obj && obj->get_type() == ObjectDefition::Type::plain_item)
	//		{
	//			int plain_item_index = obj->get_id();
	//			plain_item_yield_[plain_item_index] = it->second;
	//			((PlainItem*)obj)->add_container_id(get_id());
	//		}
	//	}
	//
	//	hunt_difficulty_ = hunt_difficulty;
	//	ZEPHYR_ASSERT(hunt_difficulty_ > 0);
	//	animal_knowledge_index_ = (new Knowledge(name + string("_animal_knowledge"), ObjectDefition::Type::animal, get_id()))->get_id();
	//}
	//
	//Pop::Pop(int count, int pop_id) : knowledge_(pop_id)
	//{
	//	pop_count_ = count;
	//	pop_id_ = pop_id;
	//}
	//
	//void Pop::tick()
	//{
	//	float days_remaining = g_world_instance_->get_days_per_tick();
	//	for (int i = 0; i < cur_frame_decisions_.size() && days_remaining > 0; i++)
	//	{
	//		string log;
	//
	//		float days_spent = cur_frame_decisions_[i].time_spent_;
	//		days_spent = min(days_remaining, days_spent);
	//
	//		days_remaining -= cur_frame_decisions_[i].action_->execute(cur_frame_decisions_[i].params_, this, days_spent, log);
	//		action_logs_.push_back(log);
	//
	//		if (days_remaining > g_world_instance_->get_days_per_tick())
	//		{
	//			int a = 5;
	//		}
	//	}
	//
	//	if (days_remaining > 0)
	//	{
	//		for (int i = 0; i < free_time_decisions_.size(); i++)
	//		{
	//			string log;
	//			float days_to_do = free_time_decisions_[i].priority_weight_ * days_remaining;
	//			free_time_decisions_[i].action_->execute(free_time_decisions_[i].params_, this, days_to_do, log);
	//			action_logs_.push_back(log);
	//		}
	//	}
	//
	//	cur_frame_decisions_.clear();
	//	free_time_decisions_.clear();
	//
	//	knowledge_.tick();
	//
	//	const float daily_pop_increase = 0.5 * 0.3 * 0.1 * 0.03;
	//	float percentage = 1.0f + (g_world_instance_->get_random_value() * 2.0f - 1.0f) * 0.25f;
	//	pop_count_ += percentage * daily_pop_increase;
	//
	//	for (int i = 0; i < needs_.size(); i++)
	//	{
	//		needs_[i]->apply(this);
	//	}
	//}
	//
	//void Pop::decide()
	//{
	//	//fetch all needs
	//	vector<pair<ObjectDefition::ObjectProperty, float>> item_subset_to_amount;
	//	get_needs_with_lacking_items(item_subset_to_amount);
	//
	//	//fetch actions to solve these
	//	vector<pair<int, float>> known_actions_with_exp;
	//	knowledge_.get_knowledge_ids_with_type(ObjectDefition::Type::action, known_actions_with_exp);
	//
	//	bool any_need_is_unsatisfied = false;
	//
	//	typedef tuple<Action*, ObjectDefition::ObjectProperty, float> ActionDef;
	//	for (int i = 0; i < item_subset_to_amount.size(); i++)
	//	{
	//		ObjectDefition::ObjectProperty cur_subset = item_subset_to_amount[i].first;
	//		vector<ActionDef> chosen_actions;
	//		for (int action_id = 0; action_id < known_actions_with_exp.size(); action_id++)
	//		{
	//			Knowledge *knowledge = ((Knowledge*)g_world_instance_->get_object_by_index(known_actions_with_exp[action_id].first));
	//			int action_index = knowledge->get_associated_item_id();
	//			Action *action = ((Action*)g_world_instance_->get_object_by_index(action_index));
	//
	//			float value = action->get_yield_per_hour_of_property_type(cur_subset, knowledge_, current_tile_, get_inventory());
	//
	//			if (action->has_yield_with_property_type(cur_subset))
	//			{
	//				chosen_actions.push_back(make_tuple(action,cur_subset, value * (0.5 + g_world_instance_->get_random_value() * 0.5)));
	//			}
	//		}
	//
	//		//sort wrt to efficency fo action
	//		sort(chosen_actions.begin(), chosen_actions.end(),
	//			[](const ActionDef & a, const ActionDef & b) -> bool
	//		{
	//			return get<2>(a) > get<2>(b);
	//		});
	//
	//		if (chosen_actions.size() > 0)
	//		{
	//			cur_frame_decisions_.push_back(ActionDecision(get<0>(chosen_actions[0]), {item_subset_to_amount[i].second}, -1));
	//		}
	//		else
	//		{
	//			any_need_is_unsatisfied = true;
	//		}
	//	}
	//
	//	if (any_need_is_unsatisfied == false)
	//	{
	//		int next_tile = g_world_instance_->get_random_neighbour_tile(current_tile_);
	//		cur_frame_decisions_.push_back(ActionDecision((Action*)g_world_instance_->get_object_by_name("Travel"), { (float)next_tile }, -1));
	//	}
	//
	//	//insert actions that may increase the efficency of these actions
	//	//TODO_MURAT000
	//	vector<pair<CraftingAction *, float>> crafted_item_to_efficency;
	//	for (int i = 0; i < cur_frame_decisions_.size(); i++)
	//	{
	//		Action *cur_action = cur_frame_decisions_[i].action_;
	//		const map<int, pair<float, float>> & efficency_items = cur_action->get_efficency_items();
	//		for (auto it = efficency_items.begin(); it != efficency_items.end(); it++)
	//		{
	//			if (plain_items_owned_.find(it->first) == plain_items_owned_.end())
	//			{
	//				CraftingAction *crafting_action = CraftingAction::get_action_with_crafted_id(it->first);
	//				CraftedItem *item = (CraftedItem *)g_world_instance_->get_object_by_index(it->first);
	//				float action_efficency = crafting_action->get_action_efficency(get_knowledge(), current_tile_, get_inventory());
	//				float item_efficency = it->second.first;
	//
	//				if (get_knowledge().get_knowledge(crafting_action->get_knowledge_id()) > 0.0f)
	//				{
	//					bool is_inventory_suitable = item->get_max_item_crafted_from_inventory(get_inventory()) > 0.0f;
	//
	//					if (is_inventory_suitable == false)
	//					{
	//						//TODO_MURAT000 ! gather if cant craft anything
	//					}
	//					else
	//					{
	//						//TODO_MURAT000 : revolutionary vs conservative !
	//						float weight = (min(action_efficency, 1.2f) + item_efficency) * (0.75f + g_world_instance_->get_random_value() * 0.5);
	//						crafted_item_to_efficency.push_back(make_pair(crafting_action, weight));
	//					}
	//				}
	//			}
	//		}
	//	}
	//
	//	if(crafted_item_to_efficency.size() > 0)
	//	{
	//		const float max_crafting_time = g_world_instance_->get_days_per_tick() * 0.25;
	//		float total_weight = 0;
	//		int number_of_crafts = max(1, g_world_instance_->get_random_index(crafted_item_to_efficency.size()));
	//		//select a random amount of them and assign weights
	//
	//		//sort wrt to weight
	//		sort(crafted_item_to_efficency.begin(), crafted_item_to_efficency.end(),
	//			[](const pair<CraftingAction *, float> & a, const pair<CraftingAction *, float> & b) -> bool
	//		{
	//			return a.second > b.second;
	//		});
	//
	//		for (int i = 0; i < number_of_crafts; i++)
	//		{
	//			total_weight += crafted_item_to_efficency[i].second;
	//		}
	//
	//		for (int i = 0; i < number_of_crafts; i++)
	//		{
	//			cur_frame_decisions_.insert(cur_frame_decisions_.begin(), ActionDecision(crafted_item_to_efficency[i].first, {  } , max_crafting_time * crafted_item_to_efficency[i].second / total_weight));
	//		}
	//	}
	//
	//	//observe at remaining time
	//	{
	//		static const int number_of_things_to_observe = 1 + g_world_instance_->get_random_index(3);
	//		vector<int> things_to_observe;
	//		Tile *cur_tile = g_world_instance_->get_tile(current_tile_);
	//		cur_tile->get_observable_things_in_tile(things_to_observe);
	//
	//		int count = things_to_observe.size();
	//		if (count > 0)
	//		{
	//			struct FreeActionDef
	//			{
	//				vector<float> parameters;
	//				float weight;
	//				Action *action;
	//
	//				bool operator < (const FreeActionDef& str) const
	//				{
	//					return (weight > str.weight);
	//				}
	//			};
	//			vector<FreeActionDef> actions;
	//			float total_weight = 0;
	//
	//			Action *observe_action = (Action*)g_world_instance_->get_object_by_name("Observe");
	//
	//			for (int i = 0; i < things_to_observe.size(); i++)
	//			{
	//				int thing_to_observe = things_to_observe[i];
	//				int knowledge_id = g_world_instance_->get_object_by_index(thing_to_observe)->get_associated_knowledge_id();
	//
	//				float weight = get_knowledge().get_knowledge(knowledge_id);
	//
	//				FreeActionDef new_def;
	//				new_def.parameters.push_back(thing_to_observe);
	//				new_def.weight = weight;
	//				new_def.weight *= g_world_instance_->get_random_value() * 0.4 + 0.8f;
	//				new_def.action = observe_action;
	//				actions.push_back(new_def);
	//			}
	//
	//			//sort them by weight
	//			std::sort(actions.begin(), actions.end());
	//			vector<FreeActionDef> used_actions;
	//			int random_index = g_world_instance_->get_random_index(actions.size());
	//			used_actions.push_back(actions[random_index]);
	//			actions.erase(actions.begin() + random_index);
	//
	//			if (actions.size() > 0)
	//			{
	//				used_actions.push_back(actions[0]);
	//				actions.erase(actions.begin());
	//			}
	//
	//			if (actions.size() > 0)
	//			{
	//				used_actions.push_back(actions[0]);
	//				actions.erase(actions.begin());
	//			}
	//
	//			for (int i = 0; i < used_actions.size(); i++)
	//			{
	//				total_weight += used_actions[i].weight;
	//			}
	//
	//			std::sort(used_actions.begin(), used_actions.end());
	//
	//			for (int i = 0; i < used_actions.size(); i++)
	//			{
	//				if (used_actions[i].weight > 0)
	//				{
	//					float normalized_weight = total_weight > 0 ? used_actions[i].weight / total_weight : 1.0f;
	//					if ((normalized_weight > 0.0f && normalized_weight <= 1.0f) == false)
	//					{
	//						int a = 5;
	//					}
	//					free_time_decisions_.push_back(ActionDecision(used_actions[i].action, used_actions[i].parameters, -1, normalized_weight));
	//				}
	//			}
	//		}
	//	}
	//}
	//
	//void Pop::initialize_knowledge()
	//{
	//	const float max_initial_knowledge = 0.1f;
	//	const float quantity_for_max_knowledge = 200.0f;
	//
	//	vector<pair<int, float>> items_in_tile;
	//	Tile *cur_tile = g_world_instance_->get_tile(current_tile_);
	//	cur_tile->get_items_in_tile(items_in_tile);
	//
	//	for (int i = 0; i < items_in_tile.size(); i++)
	//	{
	//		ObjectDefition *obj = g_world_instance_->get_object_by_index(items_in_tile[i].first);
	//
	//		float quantity_knowledge_gain = items_in_tile[i].second / quantity_for_max_knowledge;
	//		
	//		{
	//			int knowledge_id = obj->get_associated_knowledge_id();
	//			float knowledge_gain = max_initial_knowledge * (quantity_knowledge_gain * 0.5 + g_world_instance_->get_random_value() * 0.5);
	//			add_knowledge(knowledge_id, knowledge_gain);
	//		}
	//
	//		if (obj->has_property(ObjectDefition::huntable))
	//		{
	//			HuntAction *action = HuntAction::get_action_with_hunted_id(obj->get_id());
	//			int knowledge_id = action->get_hunting_knowledge_id();
	//			float knowledge_gain = max_initial_knowledge * (quantity_knowledge_gain * 0.5 + g_world_instance_->get_random_value() * 0.5) * 0.7;
	//			add_knowledge(knowledge_id, knowledge_gain);
	//		}
	//
	//		if (obj->has_property(ObjectDefition::gatherable))
	//		{
	//			GatherAction *action = GatherAction::get_action_with_gathered_id(obj->get_id());
	//			int knowledge_id = action->get_gathering_knowledge_id();
	//			float knowledge_gain = max_initial_knowledge * (quantity_knowledge_gain * 0.5 + g_world_instance_->get_random_value() * 0.5) * 0.7;
	//			add_knowledge(knowledge_id, knowledge_gain);
	//		}
	//	}
	//}
	//
	//void Pop::give_plain_items(vector<pair<int, float>> &items)
	//{
	//	for (int i = 0; i < items.size(); i++)
	//	{
	//		auto it = plain_items_owned_.find(items[i].first);
	//		if (it != plain_items_owned_.end())
	//		{
	//			it->second += items[i].second;
	//		}
	//		else
	//		{
	//			plain_items_owned_[items[i].first] = items[i].second;
	//		}
	//	}
	//}
	//
	//void Pop::give_plain_item(int item_index, float amount)
	//{
	//	auto it = plain_items_owned_.find(item_index);
	//	if (it != plain_items_owned_.end())
	//	{
	//		it->second += amount;
	//	}
	//	else
	//	{
	//		plain_items_owned_[item_index] = amount;
	//	}
	//}
	//
	//void Pop::set_current_tile(int index)
	//{
	//	if (current_tile_ != -1)
	//	{
	//		g_world_instance_->get_tile(current_tile_)->remove_living_pop(this->pop_id_);
	//	}
	//	current_tile_ = index;
	//	if (current_tile_ != -1)
	//	{
	//		g_world_instance_->get_tile(current_tile_)->add_living_pop(this->pop_id_);
	//	}
	//}
	//
	//void Pop::add_knowledge(int knowledge_index, float amount)
	//{
	//	knowledge_.add_knowledge(knowledge_index, amount);
	//}
	//
	//float Evolution::Pop::get_total_item_of_type(ObjectDefition::ObjectProperty subset) const
	//{
	//	float amount_owned = 0;
	//	for (auto it = plain_items_owned_.begin(); it != plain_items_owned_.end(); it++)
	//	{
	//		if (g_world_instance_->get_object_by_index(it->first)->has_property(subset))
	//		{
	//			amount_owned += it->second;
	//		}
	//	}
	//
	//	return amount_owned;
	//}
	//
	//void Pop::get_needs_with_lacking_items(vector<pair<ObjectDefition::ObjectProperty, float>>& item_subset_to_amount) const
	//{
	//	for (int i = 0; i < needs_.size(); i++)
	//	{
	//		Need *cur_need = needs_[i];
	//		bool need_to_act = false;
	//		int needed_plain_item_index = -1;
	//		float needed_amount = -1;
	//
	//		const map<ObjectDefition::ObjectProperty, float>& req = needs_[i]->get_item_requirements();
	//		for (auto it = req.begin(); it != req.end(); it++)
	//		{
	//			needed_amount = (it->second * pop_count_);
	//			float amount_in_inventory = get_total_item_of_type(it->first);
	//			if (amount_in_inventory < needed_amount)
	//			{
	//				item_subset_to_amount.push_back(make_pair(it->first, needed_amount));
	//			}
	//		}
	//	}
	//}
	//
	//int Pop::get_current_tile() const
	//{
	//	return current_tile_;
	//}
	//
	//const KnowledgeHistory & Pop::get_knowledge() const
	//{
	//	return knowledge_;
	//}
	//
	//float Pop::get_plain_item_amount(int index) const
	//{
	//	auto it = plain_items_owned_.find(index);
	//	if (it != plain_items_owned_.end())
	//	{
	//		return it->second;
	//	}
	//	else
	//	{
	//		return 0.0f;
	//	}
	//}
	//
	//float Pop::get_pop_count() const
	//{
	//	return pop_count_;
	//}
	//
	//void Pop::render_gui()
	//{
	//	ImGui::Begin("Pop Data");
	//	ImGui::Text("Pop count %f", pop_count_);
	//	ImGui::Text("Tile Index %d", current_tile_);
	//
	//	if (ImGui::TreeNode("Possessions"))
	//	{
	//		for (auto it = plain_items_owned_.begin(); it != plain_items_owned_.end(); it++)
	//		{
	//			ImGui::Text("Item (%s) amount (%f)", g_world_instance_->get_object_by_index(it->first)->get_name().c_str(), it->second);
	//		}
	//		ImGui::TreePop();
	//	}
	//	if (ImGui::TreeNode("Knowledge"))
	//	{
	//		knowledge_.render_gui();
	//		ImGui::TreePop();
	//	}
	//	if (ImGui::TreeNode("Logs"))
	//	{
	//		for (int i = action_logs_.size() - 1; i >= 0; i--)
	//		{
	//			string log_tree_node = Utilities::formatted_string("Log%d", i);
	//			if (ImGui::TreeNode(log_tree_node.c_str()))
	//			{
	//				ImGui::Text(action_logs_[i].c_str());
	//				ImGui::TreePop();
	//			}
	//		}
	//		ImGui::TreePop();
	//	}
	//
	//	ImGui::End();
	//}
	//
	//PlainItem::PlainItem(string name, const initializer_list<ObjectDefition::ObjectProperty> &properties) : ObjectDefition( name, ObjectDefition::Type::plain_item , properties)
	//{
	//	knowledge_index_ = (new Knowledge(name + "_knowledge", ObjectDefition::Type::plain_item, get_id()))->get_id();
	//}
	//
	//ObjectDefition::ObjectDefition(string name, Type type, const initializer_list<ObjectDefition::ObjectProperty> &properties) : type_(type) , name_(name)
	//{
	//	index_ = g_world_instance_->get_index();
	//	for (auto it = properties.begin(); it != properties.end(); it++)
	//	{
	//		properties_ = properties_ | *it;
	//	}
	//
	//	g_world_instance_->register_object_definition(this, properties);
	//}
	//
	//int ObjectDefition::get_associated_knowledge_id() const
	//{
	//	int knowledge_id = 0;
	//	switch (get_type())
	//	{
	//	case ObjectDefition::Type::animal:
	//	{
	//		const Animal *animal = (const Animal *)this;
	//		knowledge_id = animal->get_animal_knowledge_index();
	//		break;
	//	}
	//	case ObjectDefition::Type::plant:
	//	{
	//		const Plant *plant = (const Plant *)this;
	//		knowledge_id = plant->get_plant_knowledge_index();
	//		break;
	//	}
	//	case ObjectDefition::Type::plain_item:
	//	{
	//		const PlainItem *plain_item = (const PlainItem *)this;
	//		knowledge_id = plain_item->get_knowledge_index();
	//		break;
	//	}
	//	case ObjectDefition::Type::tile:
	//	{
	//		const Tile *tile_item = (const Tile *)this;
	//		knowledge_id = tile_item->get_tile_knowledge();
	//		break;
	//	}
	//	case ObjectDefition::Type::crafted_item:
	//	{
	//		const CraftedItem *crafted_item = (const CraftedItem *)this;
	//		knowledge_id = crafted_item->get_knowledge_id();
	//		break;
	//	}
	//	default:
	//		ZEPHYR_ASSERT(false);
	//	}
	//
	//	return knowledge_id;
	//}
	//
	//Knowledge::Knowledge(string name, ObjectDefition::Type knowledge_type, int associated_item_id, vector<tuple<int, float, float, float>> req_knowledge)
	//	: ObjectDefition(name, ObjectDefition::Type::knowledge)
	//{
	//	ZEPHYR_ASSERT(knowledge_type != ObjectDefition::Type::knowledge);
	//	knowledge_type_ = knowledge_type;
	//	associated_item_id_ = associated_item_id;
	//
	//	for (auto it = req_knowledge.begin(); it != req_knowledge.end(); it++)
	//	{
	//		total_knowledge_req_weight_ += get<1>(*it);
	//		int knowledge_id_of_required_item = get<0>(*it);
	//		Knowledge *cur_knowledge = (Knowledge *)g_world_instance_->get_object_by_index(knowledge_id_of_required_item);
	//		cur_knowledge->add_discoverable_knowledge_id(get_id());
	//
	//		discovery_knowledge_reqs_.insert(make_pair(knowledge_id_of_required_item, make_tuple(get<1>(*it), get<2>(*it), get<3>(*it))));
	//	}
	//}
	//
	//void Knowledge::add_discoverable_knowledge_id(int id)
	//{
	//	ZEPHYR_ASSERT(g_world_instance_->get_object_by_index(id)->get_type() == ObjectDefition::Type::knowledge)
	//	{
	//		Knowledge *knowledge = (Knowledge *)g_world_instance_->get_object_by_index(id);
	//		ZEPHYR_ASSERT(knowledge->get_knowledge_type() == ObjectDefition::Type::crafted_item)
	//		{
	//			discoverable_knowledge_ids_.push_back(id);
	//		}
	//	}
	//}
	//
	//void Knowledge::trigger_discovery(KnowledgeHistory & history)
	//{
	//	for (int i = 0; i < discoverable_knowledge_ids_.size(); i++)
	//	{
	//		int discoverable_id = discoverable_knowledge_ids_[i];
	//		Knowledge* discoverable_knowledge = (Knowledge*)(g_world_instance_->get_object_by_index(discoverable_id));
	//
	//		float current_knowledge = history.get_knowledge(discoverable_id);
	//		if (current_knowledge < 0.1f)
	//		{
	//			float chance = discoverable_knowledge->get_chance_of_knowledge_discovery(history);
	//			float random_value = g_world_instance_->get_random_value();
	//
	//			if (random_value < chance)
	//			{
	//				const float base_discover_exp_gained = 0.01f * (0.5 + chance);
	//
	//				history.add_knowledge(discoverable_id, base_discover_exp_gained);
	//			}
	//		}
	//	}
	//
	//}
	//
	//Need::Need(string name, initializer_list<pair<ObjectDefition::ObjectProperty, float>> daily_requirement, float priority) : ObjectDefition(name, ObjectDefition::Type::need)
	//{
	//	prioirty_ = priority;
	//	for (auto it = daily_requirement.begin(); it != daily_requirement.end(); it++)
	//	{
	//		subset_requirements_.insert(make_pair(it->first, it->second));
	//	}
	//}
	//
	//void Need::apply(Pop * pop)
	//{
	//	//TODO_MURAT000
	//	//for (auto it = plain_item_requirements_.begin(); it != plain_item_requirements_.end(); it++)
	//	//{
	//	//	float amount_needed = pop->get_pop_count() * it->second;
	//	//	if (pop->get_plain_item_amount(it->first) > amount_needed)
	//	//	{
	//	//		pop->give_plain_item(it->first, -amount_needed);
	//	//	}
	//	//	else
	//	//	{
	//	//		//TODO_MURAT : penalty for not handling
	//	//	}
	//	//}
	//}
	//
	//Geography::Geography(const std::vector<VoronoiSolver::VoronoiSite>& voronoi_cells)
	//{
	//	climates_.push_back(new Climate("Sea", {}, -1, -0.25, D3DXVECTOR3(0, 0, 128), 0.0f));
	//	climates_.push_back(new Climate("Shore", {make_pair("sheep",50), make_pair("tree",2) }, -0.25, 0, D3DXVECTOR3(0, 255, 255), 0));
	//	climates_.push_back(new Climate("Sand", { make_pair("sheep",70) } , 0, 0.125, D3DXVECTOR3(240, 240, 64), 0));
	//	climates_.push_back(new Climate("Grass", { make_pair("sheep",100), make_pair("tree",5) , make_pair("stone", 10) } , 0.125, 0.3750, D3DXVECTOR3(32, 160, 0), 0));
	//	climates_.push_back(new Climate("Forest", { make_pair("deer",80) , make_pair("sheep",30) , make_pair("tree",25), make_pair("stone", 50) } , 0.375, 0.75, D3DXVECTOR3(13, 60, 0), 0.33));
	//	climates_.push_back(new Climate("Rocky", { make_pair("deer",50) , make_pair("tree",12), make_pair("stone", 150) } , 0.75, 1, D3DXVECTOR3(128, 128, 128), 0.66));
	//	climates_.push_back(new Climate("Mountain", { make_pair("deer",10) , make_pair("tree",6) , make_pair("stone", 250) }, 1, 5, D3DXVECTOR3(255, 255, 255), 1));
	//
	//	for (int i = 0; i < voronoi_cells.size(); i++)
	//	{
	//		float height = voronoi_cells[i].point.z;
	//		Climate *climate = nullptr;
	//		for (int j = 0; j < climates_.size(); j++)
	//		{
	//			if (climates_[j]->get_min() <= height && climates_[j]->get_max() >= height)
	//			{
	//				climate = climates_[j];
	//				break;
	//			}
	//		}
	//
	//		tiles_.insert(make_pair(i, new Tile(D3DXVECTOR3(voronoi_cells[i].point.x, voronoi_cells[i].point.y, height), i, voronoi_cells[i].neighbours, climate)));
	//		number_of_tiles_++;
	//	}
	//}
	//
	//Climate::Climate(string name, initializer_list<pair<string, int>> things_in_tile,
	//	float min_height, float max_height, D3DXVECTOR3 rgb, float hunting_difficulty) : ObjectDefition(name, ObjectDefition::Type::climate)
	//{
	//	for (auto it = things_in_tile.begin(); it != things_in_tile.end(); it++)
	//	{
	//		ObjectDefition *obj = g_world_instance_->get_object_by_name(it->first);
	//		if (obj)
	//		{
	//			if(obj->get_type() == ObjectDefition::Type::animal)
	//			{
	//				animal_pop_.insert(make_pair(obj->get_id(), it->second));
	//			}
	//			else if (obj->get_type() == ObjectDefition::Type::plant)
	//			{
	//				fauna_.insert(make_pair(obj->get_id(), it->second));
	//			}
	//			else if (obj->get_type() == ObjectDefition::Type::plain_item)
	//			{
	//				things_in_tile_.insert(make_pair(obj->get_id(), it->second));
	//			}
	//		}
	//	}
	//
	//
	//	min_height_ = min_height;
	//	max_height_ = max_height;
	//	middle_height_ = (max_height_ + min_height_) * 0.5f;
	//	rgb_ = rgb;
	//	hunt_difficulty_ = hunting_difficulty;
	//}
	//
	//void Tile::get_gatherable_things_in_tile(vector<int>& things) const
	//{
	//	for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
	//	{
	//		things.push_back(it->first);
	//	}
	//
	//	for (auto it = plain_things_amount_.begin(); it != plain_things_amount_.end(); it++)
	//	{
	//		things.push_back(it->first);
	//	}
	//}
	//
	//void Tile::get_observable_things_in_tile(vector<int>& things) const
	//{
	//	for (auto it = plant_pop_.begin(); it != plant_pop_.end(); it++)
	//	{
	//		things.push_back(it->first);
	//	}
	//
	//	for (auto it = plain_things_amount_.begin(); it != plain_things_amount_.end(); it++)
	//	{
	//		things.push_back(it->first);
	//	}
	//
	//	for (auto it = animal_pop_.begin(); it != animal_pop_.end(); it++)
	//	{
	//		things.push_back(it->first);
	//	}
	//}
	//
	//void Tile::modify_thing_count(int thing_id, float amount)
	//{
	//	auto it = animal_pop_.find(thing_id);
	//	if (animal_pop_.find(thing_id) != animal_pop_.end())
	//	{
	//		it->second += amount;
	//		return;
	//	}
	//
	//	it = plant_pop_.find(thing_id);
	//	if (plant_pop_.find(thing_id) != plant_pop_.end())
	//	{
	//		it->second += amount;
	//		return;
	//	}
	//
	//	it = plain_things_amount_.find(thing_id);
	//	if (plain_things_amount_.find(thing_id) != plain_things_amount_.end())
	//	{
	//		it->second += amount;
	//		return;
	//	}
	//}
	//
	//CraftedItem::CraftedItem(string name, initializer_list<tuple<string, float, float, float>> needed_items_per_recipe, 
	//	initializer_list<tuple<string,float,float>> efficency_factor_for_actions) : ObjectDefition(name, ObjectDefition::Type::crafted_item)
	//{
	//	float total_weight = 0;
	//	for (auto it = needed_items_per_recipe.begin(); it != needed_items_per_recipe.end(); it++)
	//	{
	//		ObjectDefition *item = g_world_instance_->get_object_by_name(get<0>(*it));
	//		if (item != nullptr)
	//		{
	//			ZEPHYR_SAFE_ASSERT(item->get_type() == ObjectDefition::Type::crafted_item || item->get_type() == ObjectDefition::Type::plain_item)
	//			{
	//				int id = item->get_id();
	//				needed_items_per_recipe_.insert(make_pair(id, get<1>(*it)));
	//			}
	//		}
	//		total_weight += get<1>(*it);
	//	}
	//
	//	vector<tuple<int, float, float, float>> list;
	//	for (auto it = needed_items_per_recipe.begin(); it != needed_items_per_recipe.end(); it++)
	//	{
	//		ObjectDefition * obj = g_world_instance_->get_object_by_name(get<0>(*it));
	//		int item_knowledge_id = obj->get_associated_knowledge_id();
	//		list.push_back(make_tuple(item_knowledge_id, get<1>(*it), get<2>(*it), get<3>(*it)));
	//
	//		normalized_item_knowledge_weights_[item_knowledge_id] =  get<1>(*it) / total_weight;
	//	}
	//
	//	own_knowledge_id_ = (new Knowledge(name + string("_crafting_knowledge"), ObjectDefition::Type::crafted_item ,get_id(), list))->get_id();
	//}
	//
	//float CraftedItem::get_max_item_crafted_from_inventory(const map<int, float>& inventory) const
	//{
	//	float min_amount = 1e9;
	//
	//	for (auto it = needed_items_per_recipe_.begin(); it != needed_items_per_recipe_.end(); it++)
	//	{
	//		int item_id = it->first;
	//		float amount = it->second;
	//		auto inventory_it = inventory.find(item_id);
	//		float amount_in_inventory = inventory_it != inventory.end() ? inventory_it->second : 0;
	//
	//		min_amount = min(min_amount, amount_in_inventory / amount);
	//	}
	//
	//	return min_amount;
	//}
	//
	//float Knowledge::get_chance_of_knowledge_discovery(const KnowledgeHistory & knowledge)
	//{
	//	float chance = 0;
	//
	//	for (auto it = discovery_knowledge_reqs_.begin(); it != discovery_knowledge_reqs_.end(); it++)
	//	{
	//		int knowledge_id = it->first;
	//		float knowledge_amount = knowledge.get_knowledge(knowledge_id);
	//
	//		float weight = get<0>(it->second);
	//		float min_w = get<1>(it->second); 
	//		float max_w = get<2>(it->second);
	//
	//		if (knowledge_amount <= min_w)
	//		{
	//			return 0;
	//		}
	//
	//		float factor = knowledge_amount - min_w;
	//		chance += factor / (max_w - min_w) * (weight / total_knowledge_req_weight_);
	//	}
	//
	//	return chance;
	//}
	//
	//CraftingAction::CraftingAction(int item_id) : Action(g_world_instance_->get_object_by_index(item_id)->get_name() + string("_crafting_action"))
	//{
	//	crafting_item_id_ = item_id;
	//
	//	if (g_world_instance_->get_object_by_name("global_crafting_knowledge") == nullptr)
	//	{
	//		new Knowledge("global_crafting_knowledge", ObjectDefition::Type::action, -1, {});
	//	}
	//
	//	int crafting_item_knowledge_id = g_world_instance_->get_object_by_index(crafting_item_id_)->get_associated_knowledge_id();
	//
	//	global_crafting_knowledge_id_ = g_world_instance_->get_object_by_name("global_crafting_knowledge")->get_id();
	//	string name = g_world_instance_->get_object_by_index(crafting_item_id_)->get_name();
	//	global_crafting_knowledge_id_ = (new Knowledge(name + "_crafting_knowledge", ObjectDefition::Type::action, get_id(), { make_tuple(crafting_item_knowledge_id,1.0f,0.0f,0.75f) } ))->get_id();
	//}
	//
	//float CraftingAction::execute(std::vector<float> int_parameters, Pop * executer, float spent_days, string & log)
	//{
	//	CraftedItem *thing_to_craft = (CraftedItem *)g_world_instance_->get_object_by_index(crafting_item_id_);
	//	const map<int, float> &items_per_recipe = thing_to_craft->get_needed_items();
	//
	//	const float base_day_to_craft = 14;
	//	float days_to_craft = base_day_to_craft / get_action_efficency(executer->get_knowledge(), executer->get_current_tile(), executer->get_inventory());
	//
	//	float amount_to_craft = 0;
	//	float max_amount_crafted_via_inventory = thing_to_craft->get_max_item_crafted_from_inventory(executer->get_inventory());
	//	if (spent_days < 0)		//do as much as you want
	//	{
	//		amount_to_craft = max_amount_crafted_via_inventory;
	//	}
	//	else
	//	{
	//		amount_to_craft = spent_days / base_day_to_craft;
	//	}
	//
	//	float crafted_hours = amount_to_craft * base_day_to_craft;
	//
	//	//exp gain
	//	{
	//		const float days_spent_to_max_out_skill = 30 * 12 * 10;
	//		float exp_gained_per_day = 1.0f / days_spent_to_max_out_skill;
	//		float exp_gained = (0.5 + g_world_instance_->get_random_value()) * exp_gained_per_day * crafted_hours;
	//		executer->add_knowledge(crafting_knowledge_id_, exp_gained);
	//		executer->add_knowledge(thing_to_craft->get_associated_knowledge_id(), exp_gained * 0.5f);
	//	}
	//
	//	//item gain
	//	{
	//		executer->give_plain_item(crafting_item_id_, amount_to_craft);
	//		for (auto it = items_per_recipe.begin(); it != items_per_recipe.end(); it++)
	//		{
	//			executer->give_plain_item(it->first, -amount_to_craft * it->second);
	//		}
	//	}
	//
	//	return crafted_hours;
	//}
	//
	//void CraftingAction::get_yield_items(std::vector<int>& actions_yield) const
	//{
	//	actions_yield.push_back(crafting_item_id_);
	//}
	//
	//void CraftingAction::get_yield_items_per_hour(std::vector<pair<int, float>>& actions_yield, const KnowledgeHistory & knwoledge, int tile_index, const map<int, float>& inventory) const
	//{
	//	const float base_day_to_craft = 14;
	//	float days_to_craft = base_day_to_craft / get_action_efficency(knwoledge, tile_index, inventory);
	//
	//	actions_yield.push_back(make_pair(crafting_item_id_ ,1.0f / base_day_to_craft));
	//}
	//
	//float CraftingAction::get_action_efficency(const KnowledgeHistory & knowledge, int tile_index, const map<int, float>& inventory) const
	//{
	//	CraftedItem *thing_to_craft = (CraftedItem *)g_world_instance_->get_object_by_index(crafting_item_id_);
	//	const map<int, float> &normalized_knowledge_weights = thing_to_craft->get_normalized_item_weights();
	//
	//	//TODO_MURAT000 : efficency from items !
	//
	//	float base_efficency = 0.15f;
	//	float item_crafting_efficency = 0.8 * knowledge.get_knowledge(crafting_knowledge_id_);
	//	float global_crafting_efficency = 0.25 * knowledge.get_knowledge(global_crafting_knowledge_id_);
	//	float random_efficency = 0.4 * g_world_instance_->get_random_value();
	//	float ingredient_efficency = 0.0;
	//	for (auto it = normalized_knowledge_weights.begin(); it != normalized_knowledge_weights.end(); it++)
	//	{
	//		ingredient_efficency += knowledge.get_knowledge(it->first) * it->second;
	//	}
	//	ingredient_efficency *= 0.4f;
	//
	//	float total_efficency = base_efficency + item_crafting_efficency + global_crafting_efficency + random_efficency + ingredient_efficency;
	//	return total_efficency;
	//}
	//
	//void CraftingAction::initialize_action()
	//{
	//	vector<int> huntable_things;
	//	g_world_instance_->get_indexes_of_type(ObjectDefition::Type::crafted_item, huntable_things);
	//
	//	for (int i = 0; i < huntable_things.size(); i++)
	//	{
	//		int hunted_id = huntable_things[i];
	//		int action_id = (new CraftingAction(hunted_id))->get_id();
	//		item_index_to_crafting_action_.insert(make_pair(hunted_id, action_id));
	//	}
	//}
	//
	//CraftingAction * CraftingAction::get_action_with_crafted_id(int item)
	//{
	//	return (CraftingAction*)g_world_instance_->get_object_by_index(item_index_to_crafting_action_[item]);
	//}
	//
	//void PopCache::on_knowledge_first_learn(Knowledge * knowledge)
	//{
	//
	//
	//}
	//
	//void PopCache::on_knowledge_learn(Knowledge * knowledge)
	//{
	//	ObjectDefition *associated_item = g_world_instance_->get_object_by_index(knowledge->get_associated_item_id());
	//	ObjectDefition::Type item_type = associated_item->get_type();
	//
	//	switch (item_type)
	//	{
	//	case ObjectDefition::Type::action:
	//		on_action_learned(knowledge);
	//		break;
	//	case ObjectDefition::Type::plain_item:
	//
	//		break;
	//	case ObjectDefition::Type::animal:
	//
	//		break;
	//	case ObjectDefition::Type::plant:
	//
	//		break;
	//	case ObjectDefition::Type::crafted_item:
	//
	//		break;
	//
	//	}
	//}
	//
	//void PopCache::on_action_learned(Knowledge * knowledge)
	//{
	//	int action_id = knowledge->get_associated_item_id();
	//	Action *action_learned = (Action *)g_world_instance_->get_object_by_index(action_id);
	//	
	//	knowledge_cache_.known_actions_.insert(action_learned);
	//
	//
	//
	//
	//}
	//
	//bool PopCache::get_actions_to_satisfy_need(Pop * pop, Need *need, float need_amount, vector<pair<Action, float>>& actions_and_times)
	//{
	//	//int need_id = need->get_id();
	//	//routine_cache_.need_cache_
	//
	//	//while (true)
	//	//{
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//}
	//
	//
	//	return false;
	//}
}
