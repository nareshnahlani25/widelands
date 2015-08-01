-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Quarry",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 2,
		planks = 1
	},
	return_on_dismantle = {
		log = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Cuts blocks of granite out of rocks in the vicinity.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The quarry needs rocks to cut within the work area.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 40, 45 },
		},
	},

   aihints = {
		forced_after = 60,
		stoneproducer = true
   },

	working_positions = {
		atlanteans_stonecutter = 1
	},

   outputs = {
		"granite"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
			  -- This order is on purpose so that the productivity
			  -- drops fast once all rocks are gone.
				"call=mine_stone",
				"return=skipped"
			},
		},
		mine_stone = {
			-- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
			descname = _"quarrying granite",
			actions = {
			  -- This order is on purpose so that the productivity
			  -- drops fast once all rocks are gone.
				"worker=cut_granite",
				"sleep=25000"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Rocks",
		message = _"The stonecutter working at this quarry can’t find any rocks in his work area.",
	},
}
