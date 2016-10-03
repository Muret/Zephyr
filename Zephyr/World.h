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

	class KnowledgeHistory
	{
	public:
		KnowledgeHistory();

		void add_knowledge(int index, float amount);
		float get_knowledge(int index) const;

		void tick();

	private:
		std::map<int, float> knowledge_set_;
	};

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
			tile
		};

		ObjectDefition(string name, Type type);

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

	private:
		Type type_;
		int index_;
		string name_;
	};


	class Need : public ObjectDefition
	{
	public:
		Need(string name , initializer_list<pair<string,float>> daily_requirement, float priority);

		const map<int, float>& get_item_requirements() const
		{
			return plain_item_requirements_;
		}

		void apply(Pop *pop);

	private:
		float prioirty_;
		map<int, float> plain_item_requirements_;

	};

	class Action : public ObjectDefition
	{
	public:
		Action(string name);

		virtual float execute(std::vector<int> int_parameters, Pop *executer, float spent_hours) = 0;
	};

	class TravelAction : public Action
	{
	public:
		TravelAction() : Action("Travel") {}

		virtual float execute(std::vector<int> int_parameters, Pop *executer, float spent_hours) override;
	};

	class ObserveAction : public Action
	{
	public:
		ObserveAction() : Action("Observe") {}
		
		virtual float execute(std::vector<int> int_parameters, Pop *executer, float spent_hours) override;
	};

	class HuntAction : public Action
	{
	public:
		HuntAction();

		virtual float execute(std::vector<int> int_parameters, Pop *executer, float spent_hours) override;

	private:
		std::map<int, int> animal_index_to_hunting_knowledge_index_;
	};

	class Knowledge : public ObjectDefition
	{
	public:
		Knowledge(string name, ObjectDefition::Type knowledge_type);

	private:
		ObjectDefition::Type knowledge_type_;
	};

	class PlainItem : public ObjectDefition
	{
	public:
		PlainItem(string name);

		int get_knowledge_index() const
		{
			return knowledge_index_;
		}

	private:
		int knowledge_index_;
	};


	class Animal : ObjectDefition
	{
	public:
		Animal( string name, initializer_list<pair<string,float>> plain_item_yield, float hunt_difficulty);

		float get_item_yield(int index) const;
		float get_hunt_difficulty() const;
		void get_total_yield(int number_of_catches, vector<pair<int, float>> &total_yield) const;

		int get_animal_knowledge_index() const
		{
			return animal_knowledge_index_;
		}

		bool has_item_yield(int index) const
		{
			return plain_item_yield_.find(index) != plain_item_yield_.end();
		}

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

	private:
		map<string, float> plain_item_yield_;
		float gather_difficulty_;
		int plant_knowledge_index_;
	};

	class ActionDecision
	{
	public:
		ActionDecision(Action *act, initializer_list<int> params) : action_(act), params_(params)  {}

		Action *action_;
		vector<int> params_;
	};

	class Pop
	{
	public:
		Pop(int count);

		void tick();
		void decide();

		void give_plain_items(vector<pair<int, float>> items);
		void give_plain_item(int item_index, float amount);
		void set_current_tile(int index);
		void add_knowledge(int knowledge_index, float amount);

		int get_current_tile() const;
		const KnowledgeHistory& get_knowledge() const;
		float get_plain_item_amount(int index) const;
		int get_pop_count() const;

		void set_needs(const vector<Need*> &needs)
		{
			needs_ = needs;
		}

		bool can_inventory_saatisfy_need(Need* need) const;

	private:
		int pop_count_;
		float health_value_;

		int current_tile_;

		KnowledgeHistory knowledge_;

		std::vector<int> cur_action_indices_;
		std::map<int, float> plain_items_owned_;

		std::vector<Need*> needs_;

		vector<ActionDecision> cur_frame_decisions_;
	};

	class Tile
	{
	public:
		Tile(const D3DXVECTOR3 &center, int tile_index, vector<int> neighbours_, Climate *climate);

		void reduce_animal_pop(int animal_id, int number);
		int get_animal_population(int animal_index) const;

		void get_animal_ids_in_tile(vector<int> &animal_indeces) const;
		const D3DXVECTOR3 get_position() const
		{
			return center_;
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

	private:
		D3DXVECTOR3 center_;
		int tile_index_;
		std::vector<int> neighbours_;
		Climate *climate_;

		map<int, int> animal_pop_;
		map<int, int> plant_pop_;
	};

	class Climate : public ObjectDefition
	{
	public:
		Climate(string name, initializer_list<pair<string, int>> animal_pop, initializer_list<pair<string, int>> fauna, 
			float min_height, float max_height, D3DXVECTOR3 rgb);

		float get_min() const
		{
			return min_height_;
		}

		float get_max() const
		{
			return max_height_;
		}

		const map<int, int>& get_animal_pop() const
		{
			return animal_pop_;
		}

		const map<int, int>& get_fauna() const
		{
			return fauna_;
		}

	private:

		map<int, int> animal_pop_;
		map<int, int> fauna_;
		float min_height_;
		float max_height_;
		float middle_height_;
		D3DXVECTOR3 rgb_;
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
		float get_distance_between_tiles(int first_tile, int second_tile);
		Tile *get_tile(int tile_index) const;
		int get_index();
		void get_indexes_of_type(ObjectDefition::Type type, vector<int> &indexes);
		ObjectDefition* get_object_by_index(int index) const
		{
			auto it = index_to_object_map_.find(index);
			if (it == index_to_object_map_.end())
			{
				_ASSERT(false);
				return nullptr;
			}

			return it->second;
		}
		ObjectDefition* get_object_by_name(string name) const;
		int get_random_neighbour_tile(int tile_index) const;

		void register_object_definition(ObjectDefition *object);
		void tick();

	private:
		int last_index_;
		vector<ObjectDefition*> object_definitions_;

		map<string, int> name_to_index_map_;
		map<int, ObjectDefition::Type> index_to_type_map_;
		map<int, ObjectDefition*> index_to_object_map_;

		vector<Pop*> pops_;

		Geography* geography_;
	};


};

#endif