-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Shipyard",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		planks = 2,
		granite = 3,
		spidercloth = 2
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 2,
		spidercloth = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = no_purpose_text_yet(),
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 53, 66 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 53, 66 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 53, 66 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 53, 66 },
		}
	},

   aihints = {
		needs_water = true,
		shipyard = true,
		prohibited_till = 1500
   },

	working_positions = {
		atlanteans_shipwright = 1
	},

   inputs = {
		planks = 10,
		log = 2,
		spidercloth = 4
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=20000",
				"call=ship",
				"return=skipped"
			}
		},
		ship = {
			-- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
			descname = _"constructing a ship",
			actions = {
				"check_map=seafaring",
				"construct=atlanteans_shipconstruction buildship 6",
				"animate=working 35000",
				"return=completed"
			}
		},
	},
}
