-- =======================================================================
--                      Atlanteans Tutorial Mission 01                      
-- =======================================================================

use("aux", "coroutine")
use("aux", "table")
use("aux", "infrastructure")
use("aux", "objective_utils")
use("aux", "ui")

use("map", "water_rising")

-- ===================
-- Constants & Config
-- ===================
set_textdomain("scenario_atl01.wmf")

game = wl.Game()
map = game.map
p1 = game.players[1]
first_tower_field = map:get_field(94, 149)
second_tower_field = map:get_field(79, 150)

-- =================
-- global variables 
-- =================
expand_objective = nil 
let_the_water_rise = false -- If set to true, the water will begin to rise

use("map", "texts")

-- =================
-- Helper functions 
-- =================
-- Show one message box
function msg_box(i)
   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)

      i.field = nil -- Otherwise message box jumps back
   end

   p1:message_box(i.title, i.body, i)

   sleep(130)
end

-- Show many message boxes
function msg_boxes(boxes_descr)
   for idx,box_descr in ipairs(boxes_descr) do
      msg_box(box_descr)
   end
end

-- Add an objective
function add_obj(o)
   return p1:add_objective(o.name, o.title, o.body)
end

-- Return the total number of items in warehouses of the given
-- ware.
function count_in_warehouses(ware)
   local whs = array_combine(
      p1:get_buildings("headquarters"),
      p1:get_buildings("warehouse")
   )
   local rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_wares(ware)
   end
   return rv
end

function send_building_lost_message(f)
   p1:send_message(_"Building lost!", 
([[<rt image=tribes/atlanteans/%s/idle_00.png><p>
We lost a building to the ocean.
</rt>]]):format(f.immovable.name), { field = f })
end

-- ===============
-- Initialization
-- ===============
function initialize()
   p1:allow_buildings("all")

   -- A default headquarters
   use("tribe_atlanteans", "sc00_headquarters_medium")
   init.func(p1) -- defined in sc00_headquarters_medium
   set_textdomain("scenario_atl01.wmf")

   -- Place some buildings
   prefilled_buildings(p1,
      {"high_tower", first_tower_field.x, first_tower_field.y, 
         soldiers = { [{0,0,0,0}] = 1 }
      },
      {"high_tower", second_tower_field.x, second_tower_field.y, 
         soldiers = { [{0,0,0,0}] = 1 }
      }
   )
end

-- ==============
-- Logic Threads 
-- ==============
function intro()
   sleep(1000)

   msg_boxes(initial_messages)
   sleep(200)

   initialize()
   
   build_environment()
end

function build_warehouse_and_horsefarm()
   local fields = {
      map:get_field(96, 126),
      map:get_field(28, 91),
   }

   local fowned = nil 
   while not fowned do
      for idx, f in ipairs(fields) do
         if f.owner == p1 then
            fowned = f
            break
         end
      end
      sleep(3213)
   end
   -- Has been started from the very beginning
   expand_objective.done = true
   let_the_water_rise = true
   
   scroll_smoothly_to(fowned)
   msg_boxes(horsefarm_and_warehouse_story)

   local o = add_obj(obj_horsefarm_and_warehouse)
   while not check_for_buildings(p1, {
      horsefarm = 1, warehouse = 1,
   }) do sleep(2384) end
   o.done = true
end

function build_heavy_industries_and_mining()
   msg_boxes(heavy_industry_story)

   local o = add_obj(obj_make_heavy_industrie_and_mining)
   while not check_for_buildings(p1, {
      coalmine = 1, ironmine = 1, goldmine = 1, crystalmine = 1,
      smelting_works = 1, weaponsmithy = 1, armoursmithy = 1,
      toolsmithy = 1, dungeon = 1, labyrinth = 1,
   }) do sleep(3478) end
   o.done = true
end

function build_food_environment()
   msg_boxes(food_story_message)

   run(function()
      sleep(60000)
      run(build_heavy_industries_and_mining)
   end)

   local o = add_obj(obj_make_food_infrastructure)
   while not check_for_buildings(p1, {
      farm = 1, blackroot_farm = 1,
      sawmill = 1, well = 1, bakery = 1,
      hunters_house = 1, fishers_house = 1,
      fish_breeders_house = 1, smokery = 2,
   }) do sleep(2789) end
   o.done = true

   msg_boxes(food_story_ended_messages)
end

function make_spidercloth_production()
   while count_in_warehouses("spidercloth") > 0 do sleep(2323) end

   -- There is no spidercloth in any warehouse!
   msg_boxes(spidercloth_messages)
   local o = add_obj(obj_spidercloth_production)

   while not check_for_buildings(p1, {
      spiderfarm = 1, goldweaver = 1, ["weaving-mill"] = 1
   }) do sleep(6273) end
   o.done = true

   msg_boxes(spidercloth_story_ended_messages)

end

function build_environment()
   msg_boxes(first_briefing_messages)
   local o = add_obj(obj_ensure_build_wares_production)

   expand_objective = add_obj(obj_expand)
   
   while not check_for_buildings(p1, {
      woodcutters_house = 2,
      foresters_house = 2,
      quarry = 1,
      sawmill = 1,
   }) do sleep(3731) end
   o.done = true

   run(make_spidercloth_production)

   sleep(45000) -- Sleep a while
   run(build_food_environment)
end

function leftover_buildings()
   -- All fields with left over buildings
   local lob_fields = Set:new{
      map:get_field( 59, 86),
      map:get_field( 72, 89),
      map:get_field(111,137),
      map:get_field(121,144),
   }

   local msgs = {
      first_leftover_building_found,
      second_leftover_building_found,
      third_leftover_building_found,
   }

   while lob_fields.size > 0 and #msgs > 0 do
      for f in lob_fields:items() do
         if p1:sees_field(f) then
            scroll_smoothly_to(f)

            msg_boxes(msgs[1])

            table.remove(msgs, 1)
            lob_fields:discard(f)
            break
         end
      end
      sleep(3000)
   end
end

wr = WaterRiser:new(map:get_field(92,19))
function water_rising()
   while not let_the_water_rise do sleep(3243) end

   first_message_send = nil
   local _callback_function = function(f)
      if p1:sees_field(f) and not first_message_send then
         first_message_send = true
         run(function()
            sleep(200)
            scroll_smoothly_to(f)
            msg_boxes(field_flooded_msg)
            add_obj(obj_build_ships)
            -- TODO: check that these are completed
         end)
      end

      -- TODO: remove this
      print("In callback: ", f.x, f.y, f.immovable)
      if f.immovable then
         print(f.immovable.type)
      end
      -- end of remove

      if not f.owner == p1 then return end
      if not f.immovable then return end

      -- Flags are not so interesting
      if f.immovable.type == "flag" and 
         (f.tln.immovable and f.tln.immovable.type == "building") then
         f = f.tln 
      end

      if f.immovable.type == "building" then
         send_building_lost_message(f)
      end
   end

   wr.field_flooded_callback = _callback_function

   wr:rise(25)

end

-- TODO: field callback is not called for all fields. Check this again.

run(intro)
run(leftover_buildings)
run(build_warehouse_and_horsefarm)

run(water_rising)


