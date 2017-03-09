#ifndef _WORLD_H
#define _WORLD_H

#include "includes.h"
#include "VoronoiSolver.h"

namespace Evolution
{
	class Pop;
	class Need;
	class Action;
	class Tile;
	class WorldInstance;
	class Climate;

	extern WorldInstance *g_world_instance_;

	class ObjectDefition
	{
	public:
		enum class Type
		{
			plain_item,
			animal,
			plant,
			knowledge,
			human_need,
			action,
			need,
			climate,
			tile,
			crafted_item
		};

		enum ObjectProperty : unsigned int
		{
			huntable		= 0x00000001,
			gatherable		= 0x00000002,
			food			= 0x00000004,
			shelter			= 0x00000008,
			hunting_tool	= 0x00000010,
		};

		ObjectDefition(string name, Type type, const initializer_list<ObjectProperty> &properties = {});

		string get_name() const
		{
			return name_;
		}

		int get_id() const
		{
			return index_;
		}

		Type get_type() const
		{
			return type_;
		}

		int get_associated_knowledge_id() const;

		bool has_property(ObjectProperty prop) const
		{
			return properties_ & prop;
		}

		void register_relevant_action(Action *act)
		{
			relevant_actions_set_.insert(act);
		}

		const set<Action*> &get_relevant_actions() const
		{
			return relevant_actions_set_;
		}

	private:
		Type type_;
		int index_;
		string name_;
		unsigned int properties_;
		
		set<Action*> relevant_actions_set_;
	};

	class KnowledgeHistory
	{
	public:
		KnowledgeHistory(int pop_id);

		void add_knowledge(int index, float amount);
		float get_knowledge(int index) const;

		void tick();
		void render_gui();

		void get_knowledge_ids_with_type(ObjectDefition::Type type, vector<pair<int, float>>& ids) const;

	private:
		int pop_index_;
		std::map<int, float> knowledge_set_;
	};

	class Need : public ObjectDefition
	{
	public:
		Need(string name , initializer_list<pair<ObjectDefition::ObjectProperty,float>> daily_requirement, float priority);

		const map<ObjectDefition::ObjectProperty, float>& get_item_requirements() const
		{
			return subset_requirements_;
		}

		void apply(Pop *pop);

	private:
		float prioirty_;
		map<ObjectDefition::ObjectProperty, float> subset_requirements_;

	};

	class Action : public ObjectDefition
	{
	public:
		Action(string name);

		virtual float execute(std::vector<float> int_parameters, Pop *executer, float spent_hours, string &log) = 0;
		virtual void get_yield_items(std::vector<int> &actions_yield) const = 0;
		virtual void get_yield_items_per_hour(std::vector<pair<int, float>> &actions_yield, const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const {}
		bool has_yield_with_property_type(ObjectDefition::ObjectProperty obj_prop) const;
		float get_yield_per_hour_of_property_type(ObjectDefition::ObjectProperty obj_prop, const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const;

		virtual float get_action_efficency(const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const { return 0; }

		void add_efficency_item(string name, float factor, float cease_factor_per_day);

		const map<int, pair<float, float>> &get_efficency_items() const
		{
			return efficency_items_;
		}

		float get_item_efficency_from_inventory(const map<int, float> &inventory) const;

	private:
		map<int, pair<float,float>> efficency_items_;
	};

	class CraftingAction : public Action
	{
	public:
		CraftingAction(int item_id);

		virtual float execute(std::vector<float> int_parameters, Pop *executer, float spent_hours, string &log);
		virtual void get_yield_items(std::vector<int> &actions_yield) const;
		virtual void get_yield_items_per_hour(std::vector<pair<int, float>> &actions_yield, const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const;
		virtual float get_action_efficency(const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const;

		static void initialize_action();
		static CraftingAction* get_action_with_crafted_id(int item);

		int get_knowledge_id() const
		{
			return global_crafting_knowledge_id_;
		}

	private:
		static map<int, int> item_index_to_crafting_action_;
		int global_crafting_knowledge_id_;
		int crafting_item_id_;
		int crafting_knowledge_id_;

	};

	class TravelAction : public Action
	{
	public:
		TravelAction() : Action("Travel") {}

		virtual float execute(std::vector<float> int_parameters, Pop *executer, float spent_hours, string &log) override;
		virtual void get_yield_items(std::vector<int> &actions_yield) const override {}
		virtual void get_yield_items_per_hour(std::vector<pair<int, float>> &actions_yield, const KnowledgeHistory &knwoledge, int tile_index) const {}

		static void initialize_action();
	};

	class ObserveAction : public Action
	{
	public:
		ObserveAction() : Action("Observe") {}
		
		virtual float execute(std::vector<float> int_parameters, Pop *executer, float spent_hours, string &log) override;
		virtual void get_yield_items(std::vector<int> &actions_yield) const override {}
		virtual void get_yield_items_per_hour(std::vector<pair<int, float>> &actions_yield, const KnowledgeHistory &knwoledge, int tile_index) const {}


		static void initialize_action();
	};

	class HuntAction : public Action
	{
	public:
		HuntAction(int hunted_id);

		virtual float execute(std::vector<float> int_parameters, Pop *executer, float spent_hours, string &log) override;
		virtual void get_yield_items(std::vector<int> &actions_yield) const override;
		virtual void get_yield_items_per_hour(std::vector<pair<int, float>> &actions_yield, const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const;

		static void initialize_action();
		static HuntAction* get_action_with_hunted_id(int hunted_id);
		virtual float get_action_efficency(const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const override;
		float get_hunt_time(const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory, float random_value) const;

		int get_hunting_knowledge_id() const
		{
			return hunting_knowledge_id_;
		}



	private:
		static map<int, int> animal_index_to_hunting_action_;
		int global_hunting_knowledge_id_;
		int hunted_id_;
		int hunting_knowledge_id_;

	};

	class GatherAction : public Action
	{
	public:
		GatherAction(int gathered_index);

		virtual void get_yield_items_per_hour(std::vector<pair<int, float>> &actions_yield, const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const;
		virtual float execute(std::vector<float> int_parameters, Pop *executer, float spent_hours, string &log) override;
		virtual void get_yield_items(std::vector<int> &actions_yield) const override;
		static void initialize_action();
		float get_action_efficency(const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory) const override;
		float get_gather_time(const KnowledgeHistory &knwoledge, int tile_index, const map<int, float> &inventory, float random_value) const;

		static GatherAction* get_action_with_gathered_id(int hunted_id);

		int get_gathering_knowledge_id() const
		{
			return gathering_knowledge_id_;
		}

	private:
		static map<int, int> thing_index_to_gathering_index_;
		int global_gathering_knowledge_id_;
		int gathered_id_;
		int gathering_knowledge_id_;
	};

	class Knowledge : public ObjectDefition
	{
	public:
		Knowledge(string name, ObjectDefition::Type knowledge_type, int associated_item_id, vector<tuple<int, float, float, float>> req_knowledge = {});

		ObjectDefition::Type get_knowledge_type() const
		{
			return knowledge_type_;
		}

		void add_discoverable_knowledge_id(int id);

		void trigger_discovery(KnowledgeHistory &history);
		float get_chance_of_knowledge_discovery(const KnowledgeHistory &knowledge);

		int get_associated_item_id() const
		{
			return associated_item_id_;
		}

	private:
		int associated_item_id_;
		ObjectDefition::Type knowledge_type_;
		vector<int> discoverable_knowledge_ids_;

		map<int, tuple<float, float, float>> discovery_knowledge_reqs_;
		float total_knowledge_req_weight_;
	};

	class PlainItem : public ObjectDefition
	{
	public:
		PlainItem(string name, const initializer_list<ObjectDefition::ObjectProperty> &properties = {});

		int get_knowledge_index() const
		{
			return knowledge_index_;
		}

		void add_container_id(int id)
		{
			containers_.push_back(id);
		}

	private:
		int knowledge_index_;
		vector<int> containers_;
	};


	class CraftedItem : public ObjectDefition
	{
	public:
		CraftedItem(string name, initializer_list<tuple<string,float,float,float>> needed_items_per_recipe_, initializer_list<tuple<string, float, float>> efficency_factor_for_actions);

		float get_max_item_crafted_from_inventory(const map<int, float> &inventory) const;
		int get_knowledge_id() const
		{
			return own_knowledge_id_;
		}

		const map<int, float> &get_needed_items() const
		{
			return needed_items_per_recipe_;
		}

		const map<int, float> &get_normalized_item_weights() const
		{
			return normalized_item_knowledge_weights_;
		}


	private:
		map<int, float> needed_items_per_recipe_;
		map<int, float> normalized_item_knowledge_weights_;
		int own_knowledge_id_;
	};


	class Animal : public ObjectDefition
	{
	public:
		Animal( string name, initializer_list<pair<string,float>> plain_item_yield, float hunt_difficulty);

		float get_item_yield(int index) const;
		float get_hunt_difficulty() const;
		void get_total_yield(float number_of_catches, vector<pair<int, float>> &total_yield) const;

		int get_animal_knowledge_index() const
		{
			return animal_knowledge_index_;
		}

		bool has_item_yield(int index) const
		{
			return plain_item_yield_.find(index) != plain_item_yield_.end();
		}

		float get_item_yield_with_property(ObjectDefition::ObjectProperty prop) const;

		void get_yield_ids(vector<int> &total_yield) const;

	private:
		map<int, float> plain_item_yield_;
		float hunt_difficulty_;
		int animal_knowledge_index_;
	};

	class Plant : ObjectDefition
	{
	public:
		Plant(string name, initializer_list<pair<string, float>> plain_item_yield, float gather_difficulty);

		int get_plant_knowledge_index() const
		{
			return plant_knowledge_index_;
		}
	
		void get_yield_ids(vector<pair<int, float>> &indices) const;

	private:
		map<int, float> plain_item_yield_;
		float gather_difficulty_;
		int plant_knowledge_index_;
	};

	class ActionDecision
	{
	public:
		ActionDecision(Action *act, vector<float> params, float time_spent, float priority_weight = 0) : action_(act), params_(params) , priority_weight_(priority_weight) , time_spent_(time_spent) {}

		Action *action_;
		vector<float> params_;
		float priority_weight_;
		float time_spent_;
	};

	class Pop
	{
	public:
		Pop(int count, int pop_id);

		void tick();
		void decide();

		void initialize_knowledge();

		void give_plain_items(vector<pair<int, float>> &items);
		void give_plain_item(int item_index, float amount);
		void set_current_tile(int index);
		void add_knowledge(int knowledge_index, float amount);

		float get_total_item_of_type(ObjectDefition::ObjectProperty subset) const;
		void get_needs_with_lacking_items(vector<pair<ObjectDefition::ObjectProperty, float>> &item_subset_to_amount) const;

		int get_current_tile() const;
		const KnowledgeHistory& get_knowledge() const;
		float get_plain_item_amount(int index) const;
		float get_pop_count() const;

		void set_needs(const vector<Need*> &needs)
		{
			needs_ = needs;
		}

		void get_plain_item_ids(vector<int> &ids) const
		{
			for (auto it = plain_items_owned_.begin(); it != plain_items_owned_.end(); it++)
			{
				if (it->second > 0)
				{
					ids.push_back(it->first);
				}
			}
		}

		const std::map<int, float>& get_inventory() const
		{
			return plain_items_owned_;
		}


		int get_pop_id() const
		{
			return pop_id_;
		}

		void render_gui();

	private:
		float pop_count_;
		float health_value_;

		int pop_id_;
		int current_tile_;

		KnowledgeHistory knowledge_;

		std::vector<int> cur_action_indices_;
		std::map<int, float> plain_items_owned_;

		std::vector<Need*> needs_;

		vector<ActionDecision> cur_frame_decisions_;
		vector<ActionDecision> free_time_decisions_;

		std::vector<string> action_logs_;
	};

	class Tile : public ObjectDefition
	{
	public:
		Tile(const D3DXVECTOR3 &center, int tile_index, vector<int> neighbours_, Climate *climate);

		void reduce_animal_pop(int animal_id, int number);
		int get_animal_population(int animal_index) const;

		void get_animal_ids_in_tile(vector<int> &animal_indeces) const;
		void get_plant_ids_in_tile(vector<int> &animal_indeces) const;
		const D3DXVECTOR3 get_position() const
		{
			return center_;
		}

		void get_items_in_tile(vector<pair<int, float>> &items) const;

		void add_living_pop(int i)
		{
			living_pops_.push_back(i);
		}

		void remove_living_pop(int i)
		{
			ZEPHYR_ASSERT(living_pops_.find(i) != living_pops_.end());
		}

		void get_living_pops(vector<int> &pops)
		{
			pops = living_pops_;
		}

		void set_neighbours();

		const vector<int> &get_neighbours() const
		{
			return neighbours_;
		}

		Climate *get_climate() const
		{
			return climate_;
		}

		int get_tile_knowledge() const
		{
			return tile_knowledge_;
		}

		void get_things_in_tile(vector<int> &things_in_tile);

		void render_gui();

		void get_gatherable_things_in_tile(vector<int> &things) const;
		void get_observable_things_in_tile(vector<int> &things) const;

		void modify_thing_count(int thing_id, float amount);

	private:
		D3DXVECTOR3 center_;
		int tile_index_;
		std::vector<int> neighbours_;
		Climate *climate_;

		map<int, float> animal_pop_;
		map<int, float> plant_pop_;
		map<int, float> plain_things_amount_;

		int tile_knowledge_;

		vector<int> living_pops_;
	};

	class Climate : public ObjectDefition
	{
	public:
		Climate(string name, initializer_list<pair<string, int>> things_in_tile_, 
			float min_height, float max_height, D3DXVECTOR3 rgb, float hunting_difficulty);

		float get_min() const
		{
			return min_height_;
		}

		float get_max() const
		{
			return max_height_;
		}

		const map<int, float>& get_animal_pop() const
		{
			return animal_pop_;
		}

		const map<int, float>& get_fauna() const
		{
			return fauna_;
		}

		const map<int, float>& get_plain_things_in_tile() const
		{
			return things_in_tile_;
		}

		float get_hunting_difficulty() const
		{
			return hunt_difficulty_;
		}

	private: 

		map<int, float> animal_pop_;
		map<int, float> fauna_;
		map<int, float> things_in_tile_;

		float min_height_;
		float max_height_;
		float middle_height_;
		D3DXVECTOR3 rgb_;
		float hunt_difficulty_;
	};

	class Geography
	{
	public:
		Geography(const std::vector<VoronoiSolver::VoronoiSite> &voronoi_cells);

		Tile *get_tile(int index) const
		{
			auto it = tiles_.find(index);
			if (it != tiles_.end())
			{
				return it->second;
			}
			return nullptr;
		}

		int get_number_of_tiles() const
		{
			return number_of_tiles_;
		}

	private:
		map<int, Tile*> tiles_;
		vector<Climate*> climates_;
		int number_of_tiles_;
	};

	class WorldInstance
	{
	public:
		WorldInstance(int seed, const std::vector<VoronoiSolver::VoronoiSite> &voronoi_cells);

		float get_random_value();
		int get_random_index(int count);
		float get_distance_between_tiles(int first_tile, int second_tile);
		Tile *get_tile(int tile_index) const;
		int get_index();
		void get_indexes_of_type(ObjectDefition::Type type, vector<int> &indexes);
		ObjectDefition* get_object_by_index(int index) const
		{
			auto it = index_to_object_map_.find(index);
			if (it == index_to_object_map_.end())
			{
				ZEPHYR_ASSERT(false);
				return nullptr;
			}

			return it->second;
		}
		ObjectDefition* get_object_by_name(string name) const;
		int get_random_neighbour_tile(int tile_index) const;

		void register_object_definition(ObjectDefition *object, const initializer_list<ObjectDefition::ObjectProperty> &properties);
		void tick();

		void get_items_with_property(ObjectDefition::ObjectProperty, vector<int>&) const;

		void toggle_tile_render();

		void tick_aux();
		float get_days_per_tick() const
		{
			return days_per_tick_;
		}


		void register_new_knowledge(int pop_id, int knowledge_id, float exp);

	private:
		int last_index_;
		vector<ObjectDefition*> object_definitions_;

		map<string, int> name_to_index_map_;
		map<int, ObjectDefition::Type> index_to_type_map_;
		map<int, ObjectDefition*> index_to_object_map_;

		map<ObjectDefition::ObjectProperty, vector<int> > items_with_properties_;

		vector<Pop*> pops_;

		Geography* geography_;
		
		bool tile_render_on_;
		bool automatic_tick_;
		const float days_per_tick_;

		map<ObjectDefition*, pair<int,float>> hall_of_fame_of_pops_knowledge_;
		char hof_filter_[512];


	};



	class KnowledgeCache
	{
		set<PlainItem*> known_items_;
		set<Action*> known_actions_;
		friend class PopCache;
	};

	class ItemGainCache
	{
		ObjectDefition *item_;
		vector<pair<Action*, float>> sorted_actions_with_daily_yield_;
		friend class PopCache;
	};

	class NeedCache
	{
		ObjectDefition::Type need_type_;
		//vector<pair<ObjectDefition*, float>> sorted_items_with_daily_yield_;

		set<int> known_items_that_cover_need_;
		friend class PopCache;
	};

	class RoutineCache
	{
		map<int, NeedCache> need_cache_;
		map<int, ItemGainCache> yield_cache_;
		friend class PopCache;
	};

	class PopCache
	{
		void on_knowledge_first_learn(Knowledge *knowledge);
		void on_knowledge_learn(Knowledge *knowledge);

		void on_action_learned(Knowledge *knowledge);

		bool get_actions_to_satisfy_need(Pop *pop, Need *need, float need_amount, vector<pair<Action, float>> &actions_and_times);

		KnowledgeCache knowledge_cache_;
		RoutineCache routine_cache_;
	};


};

#endif