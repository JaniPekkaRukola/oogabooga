#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"

#define m4_identity m4_make_scale(v3(1, 1, 1))


// Function Declarations (Prototypes)
// void setup_portal(Entity* en, BiomeID current_biome, BiomeID dest);
void setup_portal(Entity* en, DimensionID current_dim, DimensionID dest, SpriteID sprite_id);
void setup_item(Entity* en, ItemID item_id);
Entity* get_ore(OreID id);
DimensionData *get_dimensionData(DimensionID);





// ESSENTIALS -------------------------------------------------------------------------------------------->
	Vector2 get_mouse_pos_in_ndc() {
		float mouse_x = input_frame.mouse_x;
		float mouse_y = input_frame.mouse_y;
		Matrix4 proj = draw_frame.projection;
		Matrix4 view = draw_frame.view;
		float window_w = window.width;
		float window_h = window.height;

		// Normalize the mouse coordinates
		float ndc_x = (mouse_x / (window_w * 0.5f)) - 1.0f;
		float ndc_y = (mouse_y / (window_h * 0.5f)) - 1.0f;

		return (Vector2) {ndc_x, ndc_y};
	}

	Vector2 get_mouse_pos_in_world_space() {
		float mouse_x = input_frame.mouse_x;
		float mouse_y = input_frame.mouse_y;
		Matrix4 proj = draw_frame.projection;
		Matrix4 view = draw_frame.view;
		float window_w = window.width;
		float window_h = window.height;

		// Normalize the mouse coordinates
		float ndc_x = (mouse_x / (window_w * 0.5f)) - 1.0f;
		float ndc_y = (mouse_y / (window_h * 0.5f)) - 1.0f;

		// Transform to world coordinates
		Vector4 world_pos = v4(ndc_x, ndc_y, 0, 1);
		world_pos = m4_transform(m4_inverse(proj), world_pos);
		world_pos = m4_transform(view, world_pos);
		// log("%f, %f", world_pos.x, world_pos.y);

		// Return as 2D vector
		return (Vector2){ world_pos.x, world_pos.y};
	}

	Vector2 get_mouse_pos_in_screen(){
		// just returns the mouse position in screen
		Vector2 pos = get_mouse_pos_in_ndc();
		return v2(pos.x * (0.5 * screen_width) + (0.5 * screen_width), pos.y * (0.5 * screen_height) + (0.5 * screen_height));
	}

	int world_pos_to_tile_pos(float world_pos) {
		return roundf(world_pos / (float)tile_width);
	}

	float tile_pos_to_world_pos(int tile_pos) {
		return ((float)tile_pos * (float)tile_width);
	}

	Vector2 round_v2_to_tile(Vector2 world_pos) {
		world_pos.x = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.x));
		world_pos.y = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.y));
		return world_pos;
	}

	float sin_breathe(float time, float rate) {	// 0 -> 1
		return (sin(time * rate) + 1.0) / 2.0;
	}

	bool almost_equals(float a, float b, float epsilon) {
		return fabs(a - b) <= epsilon;
	}

	bool animate_f32_to_target(float* value, float target, float delta_t, float rate) {
		*value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
		if (almost_equals(*value, target, 0.001f)) {
			*value = target;
			return true; // reached
		}
		return false;
	}

	void animate_v2_to_target(Vector2* value, Vector2 target, float delta_t, float rate) {
		animate_f32_to_target(&(value->x), target.x, delta_t, rate);
		animate_f32_to_target(&(value->y), target.y, delta_t, rate);
	}

	Range2f quad_to_range(Draw_Quad quad) {
		return (Range2f) {quad.bottom_left, quad.top_right};
	}

	Vector2 get_player_pos(){
		// TODO: make this better. surely this can be done without a for-loop!?
		// for (int i = 0; i < MAX_ENTITY_COUNT; i++){
		// for (int i = 0; i < world->entity_count; i++){
		// for (int i = 0; i < world->dimension.entity_count; i++){
		// 	// Entity en = world->entities[i];
		// 	Entity en = world->dimension.entities[i];
		// 	if (en.arch == ARCH_player){
		// 		return en.pos;
		// 	}
		// }
		// log_error("Couldn't get player pos\n");
		// return v2(0,0);
		// if (world_frame.player){
		// 	return world_frame.player->pos;
		// }

		if (world->player){
			// return player->en->pos;
			return world->player->en->pos;
		}
	}

	void draw_rect_with_border(Matrix4 xform_slot, Vector2 inside_size, float border_width, Vector4 slot_col, Vector4 border_col){
		// draws a rect with borders
		// input xfrom is the base xform with no border
		// NOTE: if slot_col has alpha value of < 1, the border_color WILL push through underneath. See for yourself

		// draw border
		draw_rect_xform(m4_translate(xform_slot, v3(border_width * -0.5, border_width * -0.5, 0)), v2(inside_size.x + border_width, inside_size.y + border_width), border_col);
		// draw slot
		draw_rect_xform(xform_slot, v2(inside_size.y, inside_size.y), slot_col);
	}



// 



// FUNCTIONS --------------------------------------------------------------------------------------------->

	// :ENTITY ------------------------>
	Entity* entity_create() {
		// render_list.needs_sorting = true;
		// int entity_index = -1;

		Entity* entity_found = 0;

		for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
			// Entity* existing_entity = &world->entities[i];
			Entity* existing_entity = &world->dimension->entities[i];
			if (!existing_entity->is_valid) {
				// world->entity_count++;
				world->dimension->entity_count++;
				entity_found = existing_entity;
				// entity_index = i; // capture the index of the new entity
				break;
			}
		}

		assert(entity_found, "No more free entities!");
		entity_found->is_valid = true;

		// add the new entity's index to the render list
		// if (entity_index != -1) {
		// 	add_entity_to_renderlist(&render_list, entity_index);
		// }

		return entity_found;
	}

	Entity* entity_create_to_dim(DimensionID dim) {
		Entity* entity_found = 0;

		for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
			// Entity* existing_entity = &world->dimension->entities[i];
			DimensionData* dimension = get_dimensionData(dim);
			Entity* existing_entity = &dimension->entities[i];
			if (!existing_entity->is_valid) {
				dimension->entity_count++;
				entity_found = existing_entity;
				entity_found->rendering_prio = 10;
				// &dimension->entities[i] = entity_found;
				printf("CREATED ENTITY TO DIM '%s'\t", dimension->name);
				break;
			}
		}

		assert(entity_found, "No more free entities!");
		entity_found->is_valid = true;

		return entity_found;
	}

	void entity_destroy(Entity* entity) {

		// printf("DESTROYED ENTITY %s\n", entity->name);

		// render_list.needs_sorting = true;
		int index = entity - world->dimension->entities;

		world->dimension->entity_count--;

		// move the last valid entity into the destroyed entity's place (fill the gap)
		if (index < world->dimension->entity_count) {
			world->dimension->entities[index] = world->dimension->entities[world->dimension->entity_count];
		}

		// invalidate the last entity (which was moved to fill the gap)
		memset(&world->dimension->entities[world->dimension->entity_count], 0, sizeof(Entity));
		world->dimension->entities[world->dimension->entity_count].is_valid = false;
	}

	void entity_clear(Entity* entity) {
		// render_list.needs_sorting = true;
		// printf("Clearing entity '%s'\n", entity->name);
		// world->entity_count--;
		entity->is_valid = false;
		memset(entity, 0, sizeof(Entity));
	}

	int add_biomeID_to_entity(Entity* entity, BiomeID id){
		if (entity->biome_count < BIOME_MAX){
			entity->biome_ids[entity->biome_count] = id;
			entity->biome_count++;
			return 1;
		}
		else{
			return 0; // biome_ids list is full
		}
	}


	// :PLAYER ------------------------>
	Entity* get_player() {
		// return world_frame.player;
		
		// for (int i = 0; i < MAX_ENTITY_COUNT; i++){
		// 	Entity* en = &world->dimension->entities[i];
		// 	if (en->arch == ARCH_player){
		// 		return en;
		// 	}
		// }

		return world->player->en;
	}

	Entity* get_player_en_from_current_dim() {
		for (int i = 0; i < world->dimension->entity_count; i++){
			Entity* en = &world->dimension->entities[i];
			if (en->arch == ARCH_player){
				return en;
			}
		}
	}

	// Entity* get_player_en_from_dim(DimensionID dim) {
		// 	for (int i = 0; i < DIM_MAX; i++){
		// 		DimensionData* dimension = &dimensions[i];
		// 		if (dimension->dimension_id == dim){
		// 			for (int j = 0; j < dimension->entity_count; j++){
		// 				Entity* en = &dimension->entities[j];
		// 				if (en->arch == ARCH_player){
		// 					return en;
		// 				}
		// 			}
		// 		}

		// 	}
	// }

	void sync_player_pos_between_dims(){
		Vector2 current_pos = world->player->en->pos;
		for (int i = 0; i < DIM_MAX; i++){
			DimensionData* dimension = &dimensions[i];
			if (dimension){
				for (int j = 0; j < dimension->entity_count; j++){
					Entity* en = &dimension->entities[j];
					if (en->arch == ARCH_player){
						en->pos = current_pos;
						break;
					}
				}
			}
		}
	}

	bool check_player_inventory_for_items(ItemID item, int count){
		// checks if the player has enough (count) of items (item) in their inventory
		// returns "true" if player has the items
		// returns "false" if player doesn't have the items

		for (int i = 0; i < world->player->inventory_items_count; i++){
			InventoryItemData* inventory_item = &world->player->inventory[i];
			if (inventory_item->item_id == item){
				if (inventory_item->amount >= count){
					return true;
				}
			}
		}
		return false;
	}

	int get_player_inventory_item_count(ItemID item_id){
		for (int i = 0; i < world->player->inventory_items_count; i++){
			InventoryItemData* item = &world->player->inventory[i];
			if (item->item_id == item_id){
				return item->amount;
			}
		}
		return 0;
	}


	// :BIOME ------------------------->
	BiomeData get_biome_data_from_id(BiomeID id){
		if (biomes[id].enabled){
			return biomes[id];
		}
		else{
			log_error("Failed to get biomeData @ get_biome_data_from_id. NULL\n");
		}
	}


	// :DIMENSION --------------------->
	DimensionData *get_dimensionData(DimensionID id){
		return &dimensions[id];
	}

	void add_biomes_to_dimension(DimensionID dim, BiomeID* biomes, int count){
		for (int i = 0; i < count; i++){
			int asd = sizeof(biomes);
			BiomeID biome_id = biomes[i];
			dimensions[dim].biomes[i] = biome_id;
			// printf("ADDED biomeid %d to dimension %s\n", biome_id, get_dimensionData(dim)->name);
		}
	}
	
	void add_all_biomes_to_dimensions() {
		// TODO: for loop here pls
		add_biomes_to_dimension(DIM_overworld, (BiomeID[]){BIOME_forest, BIOME_cave}, 2);
		// add_biomes_to_dimension(DIM_overworld, (BiomeID[]){BIOME_pine_forest}, 1);
		add_biomes_to_dimension(DIM_cavern, (BiomeID[]){BIOME_cave}, 1);
	}


	// :PORTAL ------------------------>
	void register_portal_pair(int id, DimensionID dim1, DimensionID dim2, Vector2 pos1, Vector2 pos2) {
		if (world->portal_pair_count < MAX_PORTAL_PAIRS) {
			world->portal_pairs[world->portal_pair_count].id = id;
			world->portal_pairs[world->portal_pair_count].dim1 = dim1;
			world->portal_pairs[world->portal_pair_count].dim2 = dim2;
			world->portal_pairs[world->portal_pair_count].pos1 = pos1;
			world->portal_pairs[world->portal_pair_count].pos2 = pos2;
			world->portal_pair_count++;
		} 
		else {
			log_error("MAX_PORTAL_PAIRS reached. @ 'register_portal_pair'\n");
	    }
	}

	void update_portal_pair(int id, DimensionID dim, Vector2 pos) {
		// NOTE: find the portal pair and use it instead of doing "world->portal_pairs[i]" every time
		for (int i = 0; i < world->portal_pair_count; i++) {
			// PortalPair* pair = &world->portal_pairs[i]; // solution here (needs testing)
			if (world->portal_pairs[i].id == id) {
				if (world->portal_pairs[i].dim1 == dim) {
					world->portal_pairs[i].pos1 = pos;
				} else if (world->portal_pairs[i].dim2 == dim) {
					world->portal_pairs[i].pos2 = pos;
				}
				return;
			}
		}
		log_error("Portal pair not found. @ 'update_portal_pair'\n");
	}

	int block_portal_creation(){
		// TODO:
		// checks if there is an entity too close to the spot where the new portal would be created
		// checks if there is a portal too close to the sport where the new portal would be created
		// returns 0 if new portal is ok to be created
		// returns 1 if entity is too close to the new portal
		// returns 2 if there is a portal too close to the new portal
		return 0;
	}

	void create_portal_to(DimensionID dest, bool create_portal_bothways){

		//  ________________________________________________________________________________________________________________________________________
		// | LOGIC:
		// | - check if portal is ok to be created (check for max_portal_count and block_portal_creation)
		// | - setup the portal entity and add it to current dimension->entities (entity_create() should do this)
		// | - add portal_data and entity settings to portal entity
		// | - register a portal pair
		// | - check if create_portal_bothways
		// | - setup the second portal and add it to the destination dimension (entity_create_to_dim() should do this)
		// | - add portal_data dn entity settings to the second portal entity
		// | - update the portal_pair with the second portal's details (this necessary? cant i just register the pair at the end of this func?)
		// |________________________________________________________________________________________________________________________________________
		

		if (world->portal_count >= MAX_PORTAL_COUNT){
			log_error("MAX PORTALS REACHED FOR WORLD\n");
			return;
		}

		world->portal_count++;

		int result = block_portal_creation();

		// this is a bad way to do this
		// FIX THIS
		// int id = get_random_int_in_range(0,1000);
		int id = world->portal_count;

		if (result == 0){
			// BiomeID current_dim = world->dimension->dimension_id;
			DimensionID current_dim = world->dimension->dimension_id;

			Entity* en = entity_create();
			setup_portal(en, current_dim, dest, get_dimensionData(dest)->portal_sprite_in);
			en->pos = get_mouse_pos_in_world_space();

			// center portal (not in use)
			// en->pos.x -= get_sprite_size(get_sprite(en->sprite_id)).y * 0.5;
			// en->pos.y -= get_sprite_size(get_sprite(en->sprite_id)).y * 0.5;

			add_biomeID_to_entity(en, world->current_biome_id);
			// add_biomeID_to_entity(en, world->dimension->biome_id);
			en->is_valid = true;
			en->portal_data.enabled = true;
			en->portal_data.id = id;	// link portals
			en->portal_data.pos = en->pos;

			// register portal pair || link portal pair
			register_portal_pair(id, current_dim, dest, en->pos, v2(0, 0));

			// // continue
			// get_biome_data_from_id(world->biome_id).portal_count += 1;
			// get_biome_data_from_id(world->biome_id).has_portals = true;
			// get_biome_data_from_id(world->biome_id).portals =;
			
			// printf("Created portal to %s\n", get_biome_data_from_id(dest).name);
			printf("Created portal to %s\n", get_dimensionData(dest)->name);

			if (create_portal_bothways){
				Entity* en2 = entity_create_to_dim(dest);
				setup_portal(en2, dest, current_dim, get_dimensionData(current_dim)->portal_sprite_in);
				en2->pos = get_mouse_pos_in_world_space();
				// en->pos.x -= 10;
				// en->pos = round_v2_to_tile(en->pos);

				// ADD DESTINATION BIOME ????!!!!
				// add_biomeID_to_entity(en, dest);


				en2->is_valid = true;
				en2->portal_data.enabled = true;
				en2->portal_data.id = id;	// link portals
				en2->portal_data.pos = en2->pos;
				update_portal_pair(id, dest, en2->pos);
				printf("Created another portal to %s\n", world->dimension->name);
			}
		}
		else if (result == 1) {
			printf("Not allowed to create a portal here. Entity too close\n");
		}
		else if (result == 2) {
			printf("Not allowed to create a portal here. Another portal too close\n");
		}
	}

	void set_portal_valid(bool valid){
		if (valid){
			for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
				// Entity* en = &world->entities[i];
				Entity* en = &world->dimension->entities[i];
				if (en->arch == ARCH_portal){
					en->is_valid = true;
					en->portal_data.enabled = true;
					printf("SET en '%s' to true \n", en->name);
					break;
				}
			}
		}
		else{
			for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
			// Entity* en = &world->entities[i];
			Entity* en = &world->dimension->entities[i];
			if (en->arch == ARCH_portal){
				en->is_valid = false;
				en->portal_data.enabled = false;
				printf("SET en '%s' to true \n", en->name);
				break;
			}
		}
		}
	}
	
	PortalData* get_portal_data(DimensionID dest, int id){
		if (dest != DIM_nil){
			for (int i = 0; i < get_dimensionData(dest)->entity_count; i++){
				Entity* en = &get_dimensionData(dest)->entities[i];
				if (en->arch == ARCH_portal){
					if (en->portal_data.id == id){
						return &en->portal_data;
					}
				}
			}
		}
		else{
			log_error("Failed to get portal data @ 'get_portal_data'. trying to get portal data from dim_nil\n");
		}
		return NULL;
	}

	PortalPair* get_portal_pair(int id){
		for(int i = 0; i < world->portal_pair_count; i++){
			PortalPair* pair = &world->portal_pairs[i];
			if (pair->id == id){
				return pair;
			}
		}
		return NULL;
	}

	// :SPRITE ------------------------>
	Sprite* get_sprite(SpriteID id) {
		if (id > 0 && id <= SPRITE_MAX){
			if (id >= 0 && id < SPRITE_MAX) {
				Sprite* sprite = &sprites[id];
				if (sprite->image) {
					return sprite;
				} else {
					return &sprites[0];
				}
			}
			return &sprites[0];
		}
		return SPRITE_nil;
	}

	Vector2 get_sprite_size(Sprite* sprite) {
		if (sprite != NULL){
			return (Vector2) {sprite->image->width, sprite->image->height};
		}
		else{
			log_error("ERROR @ get_sprite_size. sprite is NULL\n");
			return v2(0,0);
		}
	}

	SpriteID get_sprite_from_itemID(ItemID item) {
		switch (item) {
			case ITEM_pine_wood: return SPRITE_item_pine_wood; break;
			case ITEM_rock: return SPRITE_item_rock; break;
			case ITEM_twig: return SPRITE_item_twig; break;
			case ITEM_sprout: return SPRITE_item_sprout; break;
			case ITEM_berry: return SPRITE_item_berry; break;
			case ITEM_mushroom0: return SPRITE_mushroom0; break;
			case ITEM_furnace: return SPRITE_building_furnace; break;
			case ITEM_workbench: return SPRITE_building_workbench; break;
			case ITEM_chest: return SPRITE_building_chest; break;
			case ITEM_fossil0: return SPRITE_item_fossil0; break;
			case ITEM_fossil1: return SPRITE_item_fossil1; break;
			case ITEM_fossil2: return SPRITE_item_fossil2; break;
			case ITEM_ORE_iron: return SPRITE_ITEM_ore_iron; break;
			case ITEM_ORE_gold: return SPRITE_ITEM_ore_gold; break;
			case ITEM_ORE_copper: return SPRITE_ITEM_ore_copper; break;
			default: return 0; break;
		}
	}

	SpriteID get_sprite_id_from_tool(ToolID tool_id) {
		// :TOOL -------------------------->
		switch (tool_id) {
			case TOOL_pickaxe: return SPRITE_tool_pickaxe; break;
			case TOOL_axe: return SPRITE_tool_axe; break;
			case TOOL_shovel: return SPRITE_tool_shovel; break;
			default: return 0; break;
		}
	}

	SpriteID get_sprite_id_from_building(BuildingID building_id) {
		switch (building_id) {
			case BUILDING_furnace: return SPRITE_building_furnace; break;
			case BUILDING_workbench: return SPRITE_building_workbench; break;
			case BUILDING_chest: return SPRITE_building_chest; break;
			default: return 0; break;
		}
	}

	// not in use
		// string get_item_name_from_ItemID(ItemID id) {
		// 	// FIX: @pin2 im defining item names in multiple different places eg.A: here
		// 	switch (id){
		// 		case ITEM_rock:{return STR("Rock");break;}
		// 		case ITEM_sprout:{return STR("Sprout");break;}
		// 		case ITEM_pine_wood:{return STR("Pine wood");break;}
		// 		case ITEM_mushroom0:{return STR("Mushroom");break;}
		// 		case ITEM_twig:{return STR("Twig");break;}

		// 		// -> pin2 

		// 		// ores
		// 		case ITEM_ORE_iron:{return STR("Iron ore");break;}
		// 		case ITEM_ORE_gold:{return STR("Gold ore");break;}
		// 		case ITEM_ORE_copper:{return STR("Copper ore");break;}
				
		// 		case ITEM_fossil0:{return STR("Fossil 0");break;}
		// 		case ITEM_fossil1:{return STR("Fossil 1");break;}
		// 		case ITEM_fossil2:{return STR("Fossil 2");break;}
		// 		default:{return STR("failed to get item name");break;}
		// 	}
		// }
	// 


	// :INVENTORY || :ITEM ------------>
void add_item_to_inventory(ItemID item, string name, int amount, EntityArchetype arch, SpriteID sprite_id, ToolID tool_id, bool valid) {
    Player *player = world->player;

    // Check if the item already exists in the inventory
    for (int i = 0; i < ITEM_MAX; i++) {
        if (player->inventory[i].valid && player->inventory[i].item_id == item) {
            // If the item already exists, increase the amount
            player->inventory[i].amount += amount;
            return;
        }
    }

    // If the item does not exist, find the first empty slot
    for (int i = 0; i < ITEM_MAX; i++) {
        if (!player->inventory[i].valid) {
            player->inventory[i].name = name;
            player->inventory[i].amount = amount;
            player->inventory[i].arch = arch;
            player->inventory[i].sprite_id = sprite_id;
            player->inventory[i].tool_id = tool_id;
            player->inventory[i].valid = valid;
            player->inventory[i].item_id = item;
            player->inventory_items_count++;
            return;
        }
    }

    printf("Inventory FULL!\n");
}

	void add_item_to_inventory_quick(InventoryItemData* item){
		// quicker way of adding item to inventory using InventoryItemData
		if (item != NULL){
			add_item_to_inventory(item->item_id, item->name, item->amount, item->arch, item->sprite_id, item->tool_id, item->valid);
		}
		else{
			log_error("Failed to add item to inventory @ 'add_item_to_inventory_quick'\n");
		}
	}

	void delete_item_from_inventory(ItemID item_id, int amount){
		for (int i = 0; i < ITEM_MAX; i++){
			InventoryItemData* inventory_item = &world->player->inventory[i];
			if (inventory_item->item_id == item_id){
				if (inventory_item->amount > 0){
					inventory_item->amount -= amount;
				}
				if (inventory_item->amount <= 0){
					// dealloc(temp_allocator, &inventory_item); // prolly does none
					world->player->inventory[i].name.count = 0;
					world->player->inventory[i].name.data = NULL;
					world->player->inventory[i].arch = 0;
					world->player->inventory[i].sprite_id = 0;
					world->player->inventory[i].tool_id = 0;
					world->player->inventory[i].item_id = 0;
					world->player->inventory[i].valid = 0;

					// Shift items down to fill the empty slot
					for (int j = i; j < ITEM_MAX - 1; j++) {
						world->player->inventory[j] = world->player->inventory[j + 1];
					}

					// Clear the last slot after shifting
					world->player->inventory[ITEM_MAX - 1].name.count = 0;
					world->player->inventory[ITEM_MAX - 1].name.data = NULL;
					world->player->inventory[ITEM_MAX - 1].arch = 0;
					world->player->inventory[ITEM_MAX - 1].sprite_id = 0;
					world->player->inventory[ITEM_MAX - 1].tool_id = 0;
					world->player->inventory[ITEM_MAX - 1].item_id = 0;
					world->player->inventory[ITEM_MAX - 1].valid = 0;

					break;
				}
				break;
			}
		}
	}

	bool spawn_item_to_world(InventoryItemData item, Vector2 pos){
		// spawns items to worldspace and returns the success as bool

		if (item.amount <= 0 || !item.valid){
			return false;
		}

		for (int i = 0; i < item.amount; i++){
				Entity* en = entity_create();
				setup_item(en, item.item_id);
				en->pos = pos;
				en->pos.x -= i;
				en->arch = item.arch;
				en->sprite_id = item.sprite_id;
				en->tool_id = item.tool_id;
				printf("Spawned item '%s' to world\n", item.name);
		}

		return true;
	}


	// :BUILDING ---------------------->
	BuildingData get_building_data(BuildingID id) {
		return buildings[id];
	}

	string get_building_name(BuildingID id) {
		switch (id) {
			case BUILDING_furnace: return STR("Furnace"); break;
			case BUILDING_workbench: return STR("Workbench"); break;
			case BUILDING_chest: return STR("Chest"); break;
			default: return STR("Error: Missing get_building_name case."); break;
		}
	}


	// :TEXTURE ----------------------->
	Texture* get_texture(TextureID id) {
		if (id >= 0 && id < TEXTURE_MAX) {
			Texture* texture = &textures[id];
			if (texture->image) {
				return texture;
			} else {
				return &textures[0];
			}
		}
		return &textures[0];
	}

	Vector2 get_texture_size(Texture texture) {
		if (texture.image){
			return (Vector2) {texture.image->width, texture.image->height};
		}
		else{
			log_error("ERROR @ get_texture_size. texture is NULL\n");
			// return v2(0,0);
		}
	}


	// :LOOT -------------------------->
	LootTable* createLootTable() {
		// LootTable *table = (LootTable *)malloc(sizeof(LootTable));
		LootTable *table = alloc(get_heap_allocator(), sizeof(LootTable));

		table->head = NULL;
		table->itemCount = 0;
		return table;
	}

	void addItemToLootTable(LootTable *table, string *name, ItemID id, float baseDropChance) {
		// LootItem *newItem = (LootItem *)malloc(sizeof(LootItem));
		LootItem *newItem = alloc(get_heap_allocator(), sizeof(LootItem));

		newItem->name = name;
		newItem->baseDropChance = baseDropChance;
		newItem->next = table->head;
		newItem->id = id;
		table->head = newItem;
		table->itemCount++;

		printf("[LOOTTABLE]: ADDED '%s' TO LOOT TABLE, Table size = %d\n", name, table->itemCount);
	}

	void generateLoot(LootTable* table, float luckModifier, Vector2 pos) {
		// @PIN1: maybe take arch as input and decide what loot table to use based on it. so instead of taking "LootTable*" in, take "arch"
		LootItem* current = table->head;
		int x_shift = 0;		// if multiple drops. this variable shifts the items abit on the x-axis.
		while(current != NULL) {
			float adjustedChance = current->baseDropChance * (1 + luckModifier);
			if (get_random_float32() < (adjustedChance / 100.0)) {
				// printf("Dropped: %s\n", current->name);

				switch (current->id) {
					case ITEM_rock: {
						{
							Entity* en = entity_create();
							// setup_item_rock(en);
							setup_item(en, ITEM_rock);
							pos.x += x_shift;
							en->pos = pos;
						}
					} break;
					case ITEM_fossil0: {
						{
							Entity* en = entity_create();
							// setup_item_fossil0(en);
							setup_item(en, ITEM_fossil0);
							pos.x += x_shift;
							en->pos = pos;
						}
					} break;
					case ITEM_fossil1: {
						{
							Entity* en = entity_create();
							// setup_item_fossil1(en);
							setup_item(en, ITEM_fossil1);
							pos.x += x_shift;
							en->pos = pos;
						}
					} break;
					case ITEM_fossil2: {
						{
							Entity* en = entity_create();
							// setup_item_fossil2(en);
							setup_item(en, ITEM_fossil2);
							pos.x += x_shift;
							en->pos = pos;
						}
					} break;

					default: {log_error("Can't spawn an item. switch defaulted");} break;
				}

				x_shift -= 5;
			}
			current = current->next;
		}
	}


	// :ORE --------------------------->
	Entity* get_ore(OreID id) {
		// for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
		for (int i = 0; i < world->dimension->entity_count; i++) {
			// Entity* en = &world->entities[i];
			Entity* en = &world->dimension->entities[i];
			if (en->ore_id == id){
				return en;
			}
		}
		log_error("Failed to get ore @ 'get_ore'\n");
	}

	string get_ore_name(OreID id){
		switch (id){
			case ORE_iron:{return STR("Iron ore");}break;
			case ORE_gold:{return STR("Iron Gold");}break;
			case ORE_copper:{return STR("Iron Copper");}break;
			default:{return STR("Failed to get ore name");}break;
		}
	}

	// :TOOL -------------------------->
	string get_tool_name(ToolID id) {
		switch (id) {
			case TOOL_pickaxe: return STR("Pickaxe"); break;
			case TOOL_axe: return STR("Axe"); break;
			default: return STR("Error: Missing get_tool_name case."); break;
		}
	}


	// :PICKUP TEXT ------------------->
	void trigger_pickup_text(Entity en) {
		// printf("TRIGGERED PICKUP TEXT for %s\n", en.name);
		for (int i = 0; i < MAX_PICKUP_TEXTS; i++) {
			if (!pickup_texts[i].active) {
				pickup_texts[i].start_pos = v2(en.pos.x, en.pos.y);
				pickup_texts[i].end_pos = v2(en.pos.x, en.pos.y + 15);
				pickup_texts[i].elapsed_time = 0.0f;
				pickup_texts[i].active = true;
				pickup_texts[i].en = en;
				pickup_texts[i].start_alpha = 1.0f;
				pickup_texts[i].end_alpha = 0.0f;
				pickup_texts[i].duration = 1.0f;
				break;  // Exit after finding an available slot
			}
		}
	}

	void update_pickup_text(float delta_time) {
		for (int i = 0; i < MAX_PICKUP_TEXTS; i++) {
			if (!pickup_texts[i].active) continue;

			pickup_texts[i].elapsed_time += delta_time;

			// Interpolation factors
			float t = pickup_texts[i].elapsed_time / pickup_texts[i].duration;
			if (t >= 1.0f) {
				pickup_texts[i].active = false;
				continue;
			}

			// calculate position
			Vector3 current_pos = v3(
				pickup_texts[i].start_pos.x + t * (pickup_texts[i].end_pos.x - pickup_texts[i].start_pos.x), 
				pickup_texts[i].start_pos.y + t * (pickup_texts[i].end_pos.y - pickup_texts[i].start_pos.y), 
				0
			);

			// fade out
			float current_alpha = pickup_texts[i].start_alpha + t * (pickup_texts[i].end_alpha - pickup_texts[i].start_alpha);

			// color
			Vector4 color = pickup_texts[i].en.pickup_text_col;
			color.a = current_alpha;


			Matrix4 xform = m4_identity;
			xform = m4_translate(xform, current_pos);


			// Draw pickup text
			draw_text_xform(font, sprint(temp_allocator, STR("+1 %s"), pickup_texts[i].en.name), font_height, xform, v2(0.1, 0.1), color);
		}
	}


	// Dimension change animation ----->
	void update_dim_change_animation(float delta_time) {

		animation.elapsed_time += delta_time;

		// Interpolation factors
		float t = animation.elapsed_time / animation.duration;
		if (t >= 1.0f) {
			animation.active = false;
			return;
		}

		animate_v2_to_target(&animation.start_pos, animation.end_pos, delta_time, 1.0f); // 4th value controls how smooth the camera transition is to the player

		// printf("Camera pos = %.2f, %.2f\n", animation.start_pos.x, animation.start_pos.y);

		world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0)); // View zoom (zooms so pixel art is the correct size)
		world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(animation.start_pos.x, animation.start_pos.y, 0)));
		world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(view_zoom, view_zoom, 1.0)));

	}


	void trigger_dim_change_animation(Vector2 camera_pos){
		// printf("TRIGGERED DIM CHANGE ANIMATION\n");
		animation.active = true;
		animation.duration = 1.0f;
		animation.elapsed_time = 0.0f;
		// animation.start_pos = v2(0, 0);
		animation.start_pos = camera_pos;
		animation.end_pos = camera_pos;
		animation.end_pos.y -= 10;
	}
// 





// SETUPS ------------------------------------------------------------------------------------------------>

	void setup_item(Entity* en, ItemID item_id) {
		en->arch = ARCH_item;
		en->name = STR("ITEM (placeholder)");
		en->sprite_id = get_sprite_from_itemID(item_id);
		en->is_item = true;
		en->item_id = item_id;
		en->enable_shadow = true;
		en->pickup_text_col = v4(1,1,1,1);
		add_biomeID_to_entity(en, world->current_biome_id);

		// naming (maybe figure out a better solution. Or just hardcode everything but only once! @pin2)
		// #crash: happens when get_ore can't find the name of the ore. prolly not because of get_ore func but because of this switch case. Happens prolly because trying to assing the result from get_ore to an entity which doesn't exist. idk why the setups above swich dont crash. idk
		switch (item_id){
			case ITEM_rock:{en->name = STR("Rock");}break;
			case ITEM_pine_wood:{en->name = STR("Pine Wood");}break;
			case ITEM_sprout:{en->name = STR("Sprout");}break;
			case ITEM_berry:{en->name = STR("Berry");}break;
			case ITEM_mushroom0:{en->name = STR("Mushroom");}break;
			case ITEM_twig:{en->name = STR("Twig");}break;
			case ITEM_furnace:{en->name = STR("Furnace");}break;
			case ITEM_workbench:{en->name = STR("Workbench");}break;
			case ITEM_chest:{en->name = STR("Chest");}break;
			case ITEM_fossil0:{en->name = STR("Ammonite Fossil");}break;
			case ITEM_fossil1:{en->name = STR("Bone Fossil");}break;
			case ITEM_fossil2:{en->name = STR("Fang Fossil");}break;
			case ITEM_fossil3:{en->name = STR("WIP");}break;
			case ITEM_ORE_iron:{en->name = get_ore_name(ORE_iron);}break;
			case ITEM_ORE_gold:{en->name = get_ore_name(ORE_gold);}break;
			case ITEM_ORE_copper:{en->name = get_ore_name(ORE_copper);}break;
			case ITEM_TOOL_pickaxe:{en->name = STR("Pickaxe");}break;
			case ITEM_TOOL_axe:{en->name = STR("Axe");}break;
			case ITEM_TOOL_shovel:{en->name = STR("Shovel");}break;
			case ITEM_BUILDING_furnace:{en->name = STR("WTF");}break;
			case ITEM_BUILDING_workbench:{en->name = STR("WTF");}break;
			case ITEM_BUILDING_chest:{en->name = STR("WTF");}break;
			case ITEM_ingot_iron:{en->name = STR("Iron ingot");}break;
			case ITEM_ingot_gold:{en->name = STR("Iron gold");}break;
			case ITEM_ingot_copper:{en->name = STR("Iron copper");}break;

			default:{en->name = STR("case missing from 'setup_item()'");}break;
		}
	}


	void setup_player_en(Entity* en, Vector2 pos) {
		en->arch = ARCH_player;
		en->name = STR("Player");
		en->sprite_id = SPRITE_player;
		en->health = player_health;
		en->pos = pos;
		en->rendering_prio = 0;
		en->enable_shadow = true;
	}


	void setup_player() {
		Player* player = NULL;
		player = alloc(get_heap_allocator(), sizeof(Player));

		Entity* en = entity_create();
		setup_player_en(en, v2(0, 0));
		
		player->en = en;
		player->walking_speed = 50.0f;
		player->running_speed_amount = 100.0f;
		player->is_running = false;
		player->entity_selection_radius = 5.0f;
		player->item_pickup_radius = 15.0f;

		// add player to world struct
		world->player = player;
	}


	void setup_rock(Entity* en) {
		en->arch = ARCH_rock;
		en->name = STR("Rock");
		en->health = rock_health;
		// @reminder gotta remember to modify the range variale in "get_random_int_in_range" based on the amount of different variations per sprite
		int random = get_random_int_in_range(0,3);
		if (random == 0){en->sprite_id = SPRITE_rock0;}
		if (random == 1){en->sprite_id = SPRITE_rock1;en->health = 1;}		// REMINDER: different sprites can have different stats eg: health
		if (random == 2){en->sprite_id = SPRITE_rock2;}
		if (random == 3){en->sprite_id = SPRITE_rock3;}
		en->destroyable = true;
		en->rendering_prio = 0;
		en->enable_shadow = true;
		add_biomeID_to_entity(en, BIOME_forest);
		add_biomeID_to_entity(en, BIOME_cave);
	}


	void setup_pine_tree(Entity* en) {
		en->arch = ARCH_tree;
		en->name = STR("Pine tree");
		// int random = get_random_int_in_range(0,3);
		// if (random == 0){en->sprite_id = SPRITE_tree0;}
		// if (random == 1){en->sprite_id = SPRITE_tree1;}
		// if (random == 2){en->sprite_id = SPRITE_tree2;}
		// if (random == 3){en->sprite_id = SPRITE_tree3;}
		en->sprite_id = SPRITE_tree_pine;
		en->health = tree_health;
		en->destroyable = true;
		en->rendering_prio = 0;
		en->enable_shadow = true;
		add_biomeID_to_entity(en, BIOME_forest);
	}


	void setup_spruce_tree(Entity* en) {
		en->arch = ARCH_tree;
		en->name = STR("Spruce tree");
		// int random = get_random_int_in_range(0,3);
		// if (random == 0){en->sprite_id = SPRITE_tree0;}
		// if (random == 1){en->sprite_id = SPRITE_tree1;}
		// if (random == 2){en->sprite_id = SPRITE_tree2;}
		// if (random == 3){en->sprite_id = SPRITE_tree3;}
		en->sprite_id = SPRITE_tree_spruce;
		en->health = tree_health;
		en->destroyable = true;
		en->rendering_prio = 0;
		en->enable_shadow = true;
		add_biomeID_to_entity(en, BIOME_forest);
	}


	void setup_bush(Entity* en) {
		en->arch = ARCH_bush;
		en->name = STR("Bush");
		int random = get_random_int_in_range(0,1);
		if (random == 0){en->sprite_id = SPRITE_bush0;en->item_id = ITEM_berry;}
		if (random == 1){en->sprite_id = SPRITE_bush1;en->item_id = ITEM_sprout;}
		en->health = bush_health;
		en->destroyable = true;
		en->rendering_prio = 0;
		en->enable_shadow = true;
		add_biomeID_to_entity(en, BIOME_forest);
	}


	void setup_twig(Entity* en) {
		en->arch = ARCH_twig;
		en->name = STR("Twig");
		en->sprite_id = SPRITE_item_twig;
		en->health = 1;
		en->destroyable = true;
		en->item_id = ITEM_twig;
		en->rendering_prio = 0;
		en->enable_shadow = true;
		add_biomeID_to_entity(en, BIOME_forest);
	}


	void setup_mushroom(Entity* en) {
		en->arch = ARCH_mushroom;
		en->name = STR("Mushroom");
		en->sprite_id = SPRITE_mushroom0;
		en->health = 1;
		en->destroyable = true;
		en->item_id = ITEM_mushroom0;
		en->rendering_prio = 0;
		en->tool_id = TOOL_nil;
		add_biomeID_to_entity(en, BIOME_forest);
		add_biomeID_to_entity(en, BIOME_cave);
	}


	void setup_ore(Entity* en, OreID id) {
		// universal
		en->arch = ARCH_ore;
		en->enable_shadow = true;
		add_biomeID_to_entity(en, BIOME_cave);

		switch (id){
			case ORE_iron:{
				{
					en->name = STR("Iron ore");
					en->sprite_id = SPRITE_ORE_iron;
					en->health = 5;
					en->destroyable = true;
					en->item_id = ITEM_ORE_iron;
					en->ore_id = ORE_iron;
				}
			} break;
			
			case ORE_gold:{
				{
					en->name = STR("Gold ore");
					en->sprite_id = SPRITE_ORE_gold;
					en->health = 4;
					en->destroyable = true;
					en->item_id = ITEM_ORE_gold;
					en->ore_id = ORE_gold;
					
				}
			} break;

			case ORE_copper:{
				{
					en->name = STR("Copper ore");
					en->sprite_id = SPRITE_ORE_copper;
					en->health = 4;
					en->destroyable = true;
					en->item_id = ITEM_ORE_copper;
					en->ore_id = ORE_copper;
					
				}
			} break;
		}
	}


	void setup_building(Entity* en, BuildingID id) {
		// universal
		en->arch = ARCH_building;
		en->enable_shadow = true;
		en->is_item = false;
		en->building_id = id;

		switch (id) {
			case BUILDING_furnace: {
				{
					en->sprite_id = SPRITE_building_furnace;
					en->destroyable = true;
					en->health = 3;
				}
			} break;

			case BUILDING_workbench: {
				{
					en->sprite_id = SPRITE_building_workbench;
					en->destroyable = true;
					en->health = 3;
				}
			} break;
			case BUILDING_chest: {
				{
					en->sprite_id = SPRITE_building_chest;
					en->destroyable = true;
					en->health = 3;
				}
			} break;

			default: log_error("Missing building_setup case entry"); break;
		}
	}


	void setup_item_building(Entity* en, BuildingID building_id) {
		en->arch = ARCH_item;
		en->sprite_id = get_sprite_id_from_building(building_id);
		en->is_item = true;
		en->building_id = building_id;
		
		// TODO: fix this
		en->item_id = ITEM_BUILDING_furnace;

		// en->render_sprite = true;
	}


	void setup_biome(BiomeID id){
		// spawn weigth = spawn amount
		// drop chace = drop chance in %

		BiomeData* biome = 0;
		biome = alloc(get_heap_allocator(), sizeof(BiomeData));

		switch (id){
			case BIOME_forest:{
				{
					biome->name = STR("Forest");
					biome->size = v2(400, 400); // actually the biome position
					biome->enabled = true;
					biome->spawn_animals = false;
					biome->spawn_water = false;
					// biome->grass_color = v4(0.32, 0.97, 0.62, 1);
					biome->grass_color = v4(1, 1, 1, 1);
					// biome->leaves_color	= v4(0, 1, 0, 1);
					biome->ground_texture = TEXTURE_grass;

					// trees
					biome->spawn_pine_trees = true;
					biome->spawn_pine_tree_weight = 400;
					biome->spawn_spruce_trees = true;
					biome->spawn_spruce_tree_weight = 400;
					biome->spawn_birch_trees = false;
					biome->spawn_birch_tree_weight = 0;
					biome->spawn_palm_trees = false;
					biome->spawn_palm_tree_weight = 0;

					// entities
					biome->spawn_rocks = true;
					biome->spawn_rocks_weight = 75;
					biome->spawn_mushrooms = true;
					biome->spawn_mushrooms_weight = 15;
					biome->spawn_twigs = true;
					biome->spawn_twigs_weight = 25;
					biome->spawn_berries = true;
					biome->spawn_berries_weight = 20;

					// fossils
					biome->spawn_fossils = true;
					biome->fossil0_drop_chance = 5;
					biome->fossil1_drop_chance = 5;
					biome->fossil2_drop_chance = 5;
					// biome->fossil_rarity_level = 2;
				}
			} break;

			case BIOME_cave:{
				{
					biome->name = STR("Cave");
					biome->size = v2(400, 400); // actually the biome position
					biome->enabled = true;
					biome->spawn_animals = false;
					biome->spawn_water = false;
					// biome->grass_color = v4(0.32, 0.97, 0.62, 1);
					// biome->grass_color = v4(1, 0.97, 0.62, 1);
					// biome->leaves_color	= v4(0, 1, 0, 1);
					biome->ground_texture = TEXTURE_cave_floor;

					// entities
					biome->spawn_rocks = true;
					biome->spawn_rocks_weight = 75;
					biome->spawn_mushrooms = true;
					biome->spawn_mushrooms_weight = 15;
					// biome->spawn_twigs = false;
					// biome->spawn_twigs_weight = 0;

					// ores
					biome->spawn_ores = true;
					biome->spawn_ore_iron = true;
					biome->spawn_ore_gold = true;
					biome->spawn_ore_copper = true;
					biome->spawn_ore_iron_weight = 20;
					biome->spawn_ore_gold_weight = 20;
					biome->spawn_ore_copper_weight = 5;

					// fossils
					biome->spawn_fossils = true;
					biome->fossil0_drop_chance = 15;
					biome->fossil1_drop_chance = 15;
					biome->fossil2_drop_chance = 15;
					// biome->fossil_rarity_level = 2;
				}
			} break;

			case BIOME_pine_forest:{
				{
					biome->name = STR("Pine forest");
					biome->size = v2(400, -400); // actually the biome position
					// TODO: maybe make a Vector4 with biome corners
					// biome->biome_pos;
					biome->enabled = true;
					biome->spawn_animals = false;
					biome->spawn_water = false;
					biome->grass_color = v4(0.6, 0.7, 1.0, 1);
					biome->ground_texture = TEXTURE_grass;

					// entities
					biome->spawn_rocks = true;
					biome->spawn_rocks_weight = 85;
					biome->spawn_mushrooms = true;
					biome->spawn_mushrooms_weight = 35;
					// biome->spawn_twigs = false;
					// biome->spawn_twigs_weight = 0;

					// ores
					biome->spawn_ores = false;
					biome->spawn_ore_iron = false;
					biome->spawn_ore_gold = false;
					biome->spawn_ore_copper = false;
					biome->spawn_ore_iron_weight = 0;
					biome->spawn_ore_gold_weight = 0;
					biome->spawn_ore_copper_weight = 0;

					// fossils
					biome->spawn_fossils = true;
					biome->fossil0_drop_chance = 15;
					biome->fossil1_drop_chance = 15;
					biome->fossil2_drop_chance = 15;
					// biome->fossil_rarity_level = 2;
				}
			}

		}

		// add biome to biomes list
		biomes[id] = *biome;
	}


	void setup_all_biomes() {
		for (BiomeID i = 0; i < BIOME_MAX; i++){
			setup_biome(i);
		}
	}


	void setup_dimension(DimensionID id) {

		DimensionData* dimension = 0;
		dimension = alloc(get_heap_allocator(), sizeof(DimensionData));
		dimension->dimension_id = id;
		dimension->ground_color = v4(1, 1, 1, 1);

		switch (id){
			case DIM_overworld:{
				{
					dimension->name = STR("Overworld (dim)");
					dimension->portal_sprite_in = SPRITE_portal1;
					// dimension->ground_color = v4(67, 105, 58, 1);
					// dimension->portal_sprite_out = SPRITE_portal0;
					// add_biomes_to_dimension(id, (BiomeID[]){BIOME_forest, BIOME_cave}, 2);
				}
			} break;

			case DIM_cavern:{
				{
					dimension->name = STR("Cavern (dim)");
					dimension->portal_sprite_in = SPRITE_portal0;
					// dimension->ground_color = v4(30, 30, 30, 1);
					// dimension->portal_sprite_out = SPRITE_portal1;
					// add_biomes_to_dimension(id, (BiomeID[]){BIOME_cave}, 1);
				}
			} break;

			default:{}break;
		}
		dimensions[id] = *dimension;
	}


	void setup_all_dimensions() {
		for (DimensionID i = 0; i < DIM_MAX; i++){
			setup_dimension(i);
		}
	}


	void setup_portal(Entity* en, DimensionID current_dim, DimensionID dest, SpriteID sprite_id){
		en->arch = ARCH_portal;
		en->name = sprintf(get_heap_allocator(), "Portal to %s", get_dimensionData(dest)->name);
		en->sprite_id = sprite_id;
		en->destroyable = false;
		en->enable_shadow = false;
		en->portal_data.dim_destination = dest;
		en->is_selectable_by_mouse = false;
		BiomeID current_biome = get_dimensionData(current_dim)->biomes[0];
		add_biomeID_to_entity(en, current_biome);
		// add_biomeID_to_entity(en, BIOME_cave);
		en->rendering_prio = -1;

		switch (sprite_id){
			case SPRITE_portal0:{
				en->rendering_prio = -1;
			} break;
			case SPRITE_portal1:{
				en->rendering_prio = 0;
			} break;
			default:{}break;
		}

	}


// 


// :RECIPES ---------------------------------------------------------------------------------------------->

	void setup_smelting_recipes(){
		// can this be automated ????

		// Iron ingot
		furnace_recipes[ITEM_ingot_iron] = (ItemData){
			.name = STR("Iron ingot"), 
			.sprite_id = SPRITE_INGOT_iron, 
			.item_id = ITEM_ingot_iron, 
			.crafting_recipe = {{ITEM_ORE_iron, 2}},
			.crafting_recipe_count = 1, 
			.cooking_time = 2.0f
		};
		
		// Gold ingot
		furnace_recipes[ITEM_ingot_gold] = (ItemData){
			.name = STR("Gold ingot"), 
			.sprite_id = SPRITE_INGOT_gold, 
			.item_id = ITEM_ingot_copper, 
			.crafting_recipe = {{ITEM_ORE_gold, 3}},
			.crafting_recipe_count = 1, 
			.cooking_time = 5.0f
		};

		// Copper ingot
		furnace_recipes[ITEM_ingot_copper] = (ItemData){
			.name = STR("Copper ingot"), 
			.sprite_id = SPRITE_INGOT_copper, 
			.item_id = ITEM_ingot_copper, 
			.crafting_recipe = {{ITEM_ORE_copper, 4}},
			.crafting_recipe_count = 1, 
			.cooking_time = 3.0f
		};

	}

	void setup_crafting_recipes(){
		// Pickaxe
		crafting_recipes[ITEM_TOOL_pickaxe] = (ItemData){
			.name = STR("PickAxe"),
			.sprite_id = SPRITE_tool_pickaxe,
			.item_id = ITEM_TOOL_pickaxe,
			.crafting_recipe = {{ITEM_rock, 4},{ITEM_twig, 2}},
			.crafting_recipe_count = 2,
			.cooking_time = 2.0f
		};

		// Axe
		crafting_recipes[ITEM_TOOL_axe] = (ItemData){
			.name = STR("Axe"),
			.sprite_id = SPRITE_tool_axe,
			.item_id = ITEM_TOOL_axe,
			.crafting_recipe = {{ITEM_rock, 3},{ITEM_twig, 2}},
			.crafting_recipe_count = 2,
			.cooking_time = 2.0f
		};
		
		// Shovel
		crafting_recipes[ITEM_TOOL_shovel] = (ItemData){
			.name = STR("Shovel"),
			.sprite_id = SPRITE_tool_shovel,
			.item_id = ITEM_TOOL_shovel,
			.crafting_recipe = {{ITEM_rock, 2},{ITEM_twig, 2}},
			.crafting_recipe_count = 2,
			.cooking_time = 2.0f
		};
	}

	void setup_all_recipes(){
		setup_smelting_recipes();
		setup_crafting_recipes();
	}


// 




#endif