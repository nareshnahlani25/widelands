/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/soldierlist.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "wlapplication.h"
#include "wui/interactive_gamebase.h"
#include "wui/soldiercapacitycontrol.h"

using Widelands::Soldier;
using Widelands::SoldierControl;

namespace {

constexpr uint32_t kMaxColumns = 6;
constexpr uint32_t kAnimateSpeed = 300;  ///< in pixels per second
constexpr uint32_t kIconBorder = 2;

}  // namespace

/**
 * Iconic representation of soldiers, including their levels and current health.
 */
struct SoldierPanel : UI::Panel {
	using SoldierFn = boost::function<void(const Soldier*)>;

	SoldierPanel(UI::Panel& parent,
	             Widelands::EditorGameBase& egbase,
	             Widelands::Building& building);

	Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	void think() override;
	void draw(RenderTarget&) override;

	void set_mouseover(const SoldierFn& fn);
	void set_click(const SoldierFn& fn);

protected:
	void handle_mousein(bool inside) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;

private:
	Vector2i calc_pos(uint32_t row, uint32_t col) const;
	const Soldier* find_soldier(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Soldier> soldier;
		uint32_t row;
		uint32_t col;
		Vector2i pos;

		/**
		 * Keep track of how we last rendered this soldier,
		 * so that we can update when its status changes.
		 */
		/*@{*/
		uint32_t cache_level;
		uint32_t cache_health;
		/*@}*/
	};

	Widelands::EditorGameBase& egbase_;
	SoldierControl& soldiers_;

	SoldierFn mouseover_fn_;
	SoldierFn click_fn_;

	std::vector<Icon> icons_;

	uint32_t rows_;
	uint32_t cols_;

	uint32_t icon_width_;
	uint32_t icon_height_;

	int32_t last_animate_time_;
};

SoldierPanel::SoldierPanel(UI::Panel& parent,
                           Widelands::EditorGameBase& gegbase,
                           Widelands::Building& building)
   : Panel(&parent, 0, 0, 0, 0),
     egbase_(gegbase),
     soldiers_(*dynamic_cast<SoldierControl*>(&building)),
     last_animate_time_(0) {
	Soldier::calc_info_icon_size(building.owner().tribe(), icon_width_, icon_height_);
	icon_width_ += 2 * kIconBorder;
	icon_height_ += 2 * kIconBorder;

	Widelands::Quantity maxcapacity = soldiers_.max_soldier_capacity();
	if (maxcapacity <= kMaxColumns) {
		cols_ = maxcapacity;
		rows_ = 1;
	} else {
		cols_ = kMaxColumns;
		rows_ = (maxcapacity + cols_ - 1) / cols_;
	}

	set_size(cols_ * icon_width_, rows_ * icon_height_);
	set_desired_size(cols_ * icon_width_, rows_ * icon_height_);
	set_thinks(true);

	// Initialize the icons
	uint32_t row = 0;
	uint32_t col = 0;
	for (Soldier* soldier : soldiers_.present_soldiers()) {
		Icon icon;
		icon.soldier = soldier;
		icon.row = row;
		icon.col = col;
		icon.pos = calc_pos(row, col);
		icons_.push_back(icon);

		if (++col >= cols_) {
			col = 0;
			row++;
		}
	}
}

/**
 * Set the callback function that indicates which soldier the mouse is over.
 */
void SoldierPanel::set_mouseover(const SoldierPanel::SoldierFn& fn) {
	mouseover_fn_ = fn;
}

/**
 * Set the callback function that is called when a soldier is clicked.
 */
void SoldierPanel::set_click(const SoldierPanel::SoldierFn& fn) {
	click_fn_ = fn;
}

void SoldierPanel::think() {
	bool changes = false;
	uint32_t capacity = soldiers_.soldier_capacity();

	// Update soldier list and target row/col:
	std::vector<Soldier*> soldierlist = soldiers_.present_soldiers();
	std::vector<uint32_t> row_occupancy;
	row_occupancy.resize(rows_);

	// First pass: check whether existing icons are still valid, and compact them
	for (uint32_t idx = 0; idx < icons_.size(); ++idx) {
		Icon& icon = icons_[idx];
		Soldier* soldier = icon.soldier.get(egbase());
		if (soldier) {
			std::vector<Soldier*>::iterator it =
			   std::find(soldierlist.begin(), soldierlist.end(), soldier);
			if (it != soldierlist.end())
				soldierlist.erase(it);
			else
				soldier = nullptr;
		}

		if (!soldier) {
			icons_.erase(icons_.begin() + idx);
			idx--;
			changes = true;
			continue;
		}

		while (icon.row && (row_occupancy[icon.row] >= kMaxColumns ||
		                    icon.row * kMaxColumns + row_occupancy[icon.row] >= capacity))
			icon.row--;

		icon.col = row_occupancy[icon.row]++;
	}

	// Second pass: add new soldiers
	while (!soldierlist.empty()) {
		Icon icon;
		icon.soldier = soldierlist.back();
		soldierlist.pop_back();
		icon.row = 0;
		while (row_occupancy[icon.row] >= kMaxColumns)
			icon.row++;
		icon.col = row_occupancy[icon.row]++;
		icon.pos = calc_pos(icon.row, icon.col);

		// Let soldiers slide in from the right border
		icon.pos.x = get_w();

		std::vector<Icon>::iterator insertpos = icons_.begin();

		for (std::vector<Icon>::iterator icon_iter = icons_.begin(); icon_iter != icons_.end();
		     ++icon_iter) {

			if (icon_iter->row <= icon.row)
				insertpos = icon_iter + 1;

			icon.pos.x = std::max<int32_t>(icon.pos.x, icon_iter->pos.x + icon_width_);
		}

		icon.cache_health = 0;
		icon.cache_level = 0;

		icons_.insert(insertpos, icon);
		changes = true;
	}

	// Third pass: animate icons
	int32_t curtime = SDL_GetTicks();
	int32_t dt = std::min(std::max(curtime - last_animate_time_, 0), 1000);
	int32_t maxdist = dt * kAnimateSpeed / 1000;
	last_animate_time_ = curtime;

	for (Icon& icon : icons_) {
		Vector2i goal = calc_pos(icon.row, icon.col);
		Vector2i dp = goal - icon.pos;

		dp.x = std::min(std::max(dp.x, -maxdist), maxdist);
		dp.y = std::min(std::max(dp.y, -maxdist), maxdist);

		if (dp.x != 0 || dp.y != 0)
			changes = true;

		icon.pos += dp;

		// Check whether health and/or level of the soldier has changed
		Soldier* soldier = icon.soldier.get(egbase());
		uint32_t level = soldier->get_attack_level();
		level = level * (soldier->descr().get_max_defense_level() + 1) + soldier->get_defense_level();
		level = level * (soldier->descr().get_max_evade_level() + 1) + soldier->get_evade_level();
		level = level * (soldier->descr().get_max_health_level() + 1) + soldier->get_health_level();

		uint32_t health = soldier->get_current_health();

		if (health != icon.cache_health || level != icon.cache_level) {
			icon.cache_level = level;
			icon.cache_health = health;
			changes = true;
		}
	}

	if (changes) {
		Vector2i mousepos = get_mouse_position();
		mouseover_fn_(find_soldier(mousepos.x, mousepos.y));
	}
}

void SoldierPanel::draw(RenderTarget& dst) {
	// Fill a region matching the current site capacity with black
	uint32_t capacity = soldiers_.soldier_capacity();
	uint32_t fullrows = capacity / kMaxColumns;

	if (fullrows) {
		dst.fill_rect(Rectf(0.f, 0.f, get_w(), icon_height_ * fullrows), RGBAColor(0, 0, 0, 0));
	}
	if (capacity % kMaxColumns) {
		dst.fill_rect(
		   Rectf(0.f, icon_height_ * fullrows, icon_width_ * (capacity % kMaxColumns), icon_height_),
		   RGBAColor(0, 0, 0, 0));
	}

	// Draw icons
	for (const Icon& icon : icons_) {
		const Soldier* soldier = icon.soldier.get(egbase());
		if (!soldier)
			continue;

		constexpr float kNoZoom = 1.f;
		soldier->draw_info_icon(
		   icon.pos.cast<float>() + Vector2f(kIconBorder, kIconBorder), kNoZoom, false, &dst);
	}
}

Vector2i SoldierPanel::calc_pos(uint32_t row, uint32_t col) const {
	return Vector2i(col * icon_width_, row * icon_height_);
}

/**
 * Return the soldier (if any) at the given coordinates.
 */
const Soldier* SoldierPanel::find_soldier(int32_t x, int32_t y) const {
	for (const Icon& icon : icons_) {
		Recti r(icon.pos, icon_width_, icon_height_);
		if (r.contains(Vector2i(x, y))) {
			return icon.soldier.get(egbase());
		}
	}

	return nullptr;
}

void SoldierPanel::handle_mousein(bool inside) {
	if (!inside && mouseover_fn_)
		mouseover_fn_(nullptr);
}

bool SoldierPanel::handle_mousemove(
   uint8_t /* state */, int32_t x, int32_t y, int32_t /* xdiff */, int32_t /* ydiff */) {
	if (mouseover_fn_)
		mouseover_fn_(find_soldier(x, y));
	return true;
}

bool SoldierPanel::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		if (click_fn_) {
			if (const Soldier* soldier = find_soldier(x, y))
				click_fn_(soldier);
		}
		return true;
	}

	return false;
}

/**
 * List of soldiers \ref MilitarySiteWindow and \ref TrainingSiteWindow
 */
struct SoldierList : UI::Box {
	SoldierList(UI::Panel& parent, InteractiveGameBase& igb, Widelands::Building& building);

	SoldierControl& soldiers() const;

private:
	void mouseover(const Soldier* soldier);
	void eject(const Soldier* soldier);
	void set_soldier_preference(int32_t changed_to);
	void think() override;

	InteractiveGameBase& igbase_;
	Widelands::Building& building_;
	SoldierPanel soldierpanel_;
	UI::Radiogroup soldier_preference_;
	UI::Textarea infotext_;
};

SoldierList::SoldierList(UI::Panel& parent, InteractiveGameBase& igb, Widelands::Building& building)
   : UI::Box(&parent, 0, 0, UI::Box::Vertical),

     igbase_(igb),
     building_(building),
     soldierpanel_(*this, igb.egbase(), building),
     infotext_(this, _("Click soldier to send away")) {
	 add(&soldierpanel_, UI::HAlign::kHCenter);

	add_space(2);

	add(&infotext_, UI::HAlign::kHCenter);

	soldierpanel_.set_mouseover(boost::bind(&SoldierList::mouseover, this, _1));
	soldierpanel_.set_click(boost::bind(&SoldierList::eject, this, _1));

	// We don't want translators to translate this twice, so it's a bit involved.
	int w =
	   UI::g_fh1->render(
	               as_uifont((boost::format("%s ")  // We need some extra space to fix bug 724169
	                          /** TRANSLATORS: Health, Attack, Defense, Evade */
	                          % (boost::format(_(
	                                "HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u")) %
	                             8 % 8 % 8 % 8 % 8 % 8 % 8 % 8))
	                            .str()))
	      ->width();
	uint32_t maxtextwidth =
	   std::max(w, UI::g_fh1->render(as_uifont(_("Click soldier to send away")))->width());
	set_min_desired_breadth(maxtextwidth + 4);

	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);

	bool can_act = igbase_.can_act(building_.owner().player_number());
	if (upcast(Widelands::MilitarySite, ms, &building)) {
		soldier_preference_.add_button(buttons, Vector2i(0, 0),
		                               g_gr->images().get("images/wui/buildings/prefer_rookies.png"),
		                               _("Prefer Rookies"));
		soldier_preference_.add_button(buttons, Vector2i(32, 0),
		                               g_gr->images().get("images/wui/buildings/prefer_heroes.png"),
		                               _("Prefer Heroes"));
		UI::Radiobutton* button = soldier_preference_.get_first_button();
		while (button) {
			buttons->add(button);
			button = button->next_button();
		}

		soldier_preference_.set_state(0);
		if (ms->get_soldier_preference() == Widelands::MilitarySite::kPrefersHeroes) {
			soldier_preference_.set_state(1);
		}
		if (can_act) {
			soldier_preference_.changedto.connect(
			   boost::bind(&SoldierList::set_soldier_preference, this, _1));
		} else {
			soldier_preference_.set_enabled(false);
		}
	}
	buttons->add_inf_space();
	buttons->add(create_soldier_capacity_control(*buttons, igb, building), UI::HAlign::kRight);

	add(buttons, UI::HAlign::kHCenter, true);
}

SoldierControl& SoldierList::soldiers() const {
	return *dynamic_cast<SoldierControl*>(&building_);
}

void SoldierList::think() {
	// Only update the soldiers pref radio if player is spectator
	if (igbase_.can_act(building_.owner().player_number())) {
		return;
	}
	if (upcast(Widelands::MilitarySite, ms, &building_)) {
		switch (ms->get_soldier_preference()) {
		case Widelands::MilitarySite::kPrefersRookies:
			soldier_preference_.set_state(0);
			break;
		case Widelands::MilitarySite::kPrefersHeroes:
			soldier_preference_.set_state(1);
			break;
		case Widelands::MilitarySite::kNoPreference:
			soldier_preference_.set_state(-1);
			break;
		}
	}
}

void SoldierList::mouseover(const Soldier* soldier) {
	if (!soldier) {
		infotext_.set_text(_("Click soldier to send away"));
		return;
	}

	infotext_.set_text(
	   (boost::format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u")) %
	    soldier->get_health_level() % soldier->descr().get_max_health_level() %
	    soldier->get_attack_level() % soldier->descr().get_max_attack_level() %
	    soldier->get_defense_level() % soldier->descr().get_max_defense_level() %
	    soldier->get_evade_level() % soldier->descr().get_max_evade_level())
	      .str());
}

void SoldierList::eject(const Soldier* soldier) {
	uint32_t const capacity_min = soldiers().min_soldier_capacity();
	bool can_act = igbase_.can_act(building_.owner().player_number());
	bool over_min = capacity_min < soldiers().present_soldiers().size();

	if (can_act && over_min)
		igbase_.game().send_player_drop_soldier(building_, soldier->serial());
}

void SoldierList::set_soldier_preference(int32_t changed_to) {
#ifndef NDEBUG
	upcast(Widelands::MilitarySite, ms, &building_);
	assert(ms);
#endif
	igbase_.game().send_player_militarysite_set_soldier_preference(
	   building_, changed_to == 0 ? Widelands::MilitarySite::kPrefersRookies :
	                                Widelands::MilitarySite::kPrefersHeroes);
}

UI::Panel*
create_soldier_list(UI::Panel& parent, InteractiveGameBase& igb, Widelands::Building& building) {
	return new SoldierList(parent, igb, building);
}
