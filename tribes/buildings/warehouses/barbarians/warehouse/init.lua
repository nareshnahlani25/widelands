-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "barbarians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Warehouse",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 2,
		grout = 3,
		thatch_reed = 1
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 1,
		grout = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Warehouses store soldiers, wares and tools.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 60, 78 }
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 60, 78 },
			fps = 1
		}
	},

	aihints = {},

	heal_per_second = 170,
}
