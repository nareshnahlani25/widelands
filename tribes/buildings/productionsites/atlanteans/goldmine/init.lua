-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Gold Mine",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
		log = 7,
		planks = 4,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 3,
		planks = 2
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs gold ore out of the ground in mountain terrain.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 50, 56 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 50, 56 },
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 50, 56 },
		},
	},

   aihints = {
		mines = "gold",
		prohibited_till = 1200
   },

	working_positions = {
		atlanteans_miner = 3
	},

   inputs = {
		atlanteans_bread = 10,
		smoked_fish = 10,
		smoked_meat = 6
	},
   outputs = {
		"gold_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
			descname = _"mining gold",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs gold_ore",
				"consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
				"animate=working 20000",
				"mine=gold 4 100 5 2",
				"produce=gold_ore",
				"animate=working 20000",
				"mine=gold 4 100 5 2",
				"produce=gold_ore",
				"animate=working 20000",
				"mine=gold 4 100 5 2",
				"produce=gold_ore"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Gold Vein Exhausted",
		message =
			_"This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider dismantling or destroying it.",
	},
}
