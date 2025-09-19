/*
    This file is a part of MuiPlusPlus project
    https://github.com/vortigont/MuiPlusPlus

    Copyright © 2024-2025 Emil Muratov (vortigont)

    MuiPlusPlus is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MuiPlusPlus is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MuiPlusPlus.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "muiplusplus.hpp"
#include <algorithm>
#include <cstdio>
#include <vector>

/*
void MuiPage::add(MuiItem_pt&& item){
  // remove exiting items with specified id if present
  auto i = std::find_if(container.cbegin(), container.cend(), muipp::MatchLabel<MuiPage_pt>(item->getid));
  if ( i != container.cend() ){
    container.erase(i);
  }
  container.emplace_back(item);
  // invalidate iterators
  currentItem = container.begin();

}

void MuiPlusPlus::add(MuiPage_pt&& page){
  // remove exiting page with specified id if present
  auto i = std::find_if(container.cbegin(), container.cend(), muipp::MatchLabel<MuiPage_pt>(page->getid));
  if ( i != container.cend() ){
    container.erase(i);
  }
  container.emplace_back(page);
  // invalidate iterators
  currentItem = prevItem = container.begin();
}
*/

void MuiPage::removeItem(muiItemId item_id){
  // if we attempt to erase current item, then iterator must be invalidated
  if ( currentItem != items.end() && (*currentItem)->id == item_id ){
    currentItem = items.end();
    itm_selected = false;
  }
  // Replace std::erase_if with remove_if + erase
  items.erase(
    std::remove_if(items.begin(), items.end(), muipp::MatchID<MuiItem_pt>(item_id)),
    items.end()
  );
}


MuiPlusPlus::MuiPlusPlus(){
  // invalidate iterator
  currentPage = pages.end();
}
/*
MuiPlusPlus::~MuiPlusPlus(){
  Serial.println("~MuiPlusPlus d-tor");
  // invalidate iterator
  currentPage = pages.end();
  pages.clear();
  items.clear();
}
*/
muiItemId MuiPlusPlus::makePage(const char* name, muiItemId parent, item_opts options){
  ++_pages_index;
  // printf("makePage %u %s, parent %u\n", _pages_index, name, parent);
  pages.emplace_back(_pages_index, name, parent, options);
  return _pages_index;
}

mui_err_t MuiPlusPlus::addMuippItem(MuiItem_pt item, muiItemId page_id){
  //Serial.printf("Adding item %u, page %u\n", item->id, page_id);
  muiItemId item_id(item->id);    // this must a copy!
  auto i = std::find_if(items.cbegin(), items.cend(), muipp::MatchID<MuiItem_pt>(item_id));
  if ( i != items.cend() ){
      // printf("item:%u already exist!\n", item->id);
    return mui_err_t::id_exist;
  }

  // move item to container
  items.emplace_back(std::move(item));
  
  // link item with the specified page
  if (page_id){
    mui_err_t err = addItemToPage(item_id, page_id);
    if (err != mui_err_t::ok) return err;
  }

  return mui_err_t::ok;
}

mui_err_t MuiPlusPlus::addItemToPage(muiItemId item_id, muiItemId page_id){
  if (!item_id || !page_id) return mui_err_t::id_err;

  // check if such page exist
  auto p = _page_by_id(page_id);
  if ( p == pages.end() ){
    // printf("page:%u not found\n", page_id);
    return mui_err_t::id_err;
  }

  auto i = _item_by_id(item_id);
  if ( i == items.end() ){
    // printf("item:%u not found\n", item_id);
    return mui_err_t::id_err;
  }
//
  (*p).items.emplace_back((*i));
  // printf("bound item:%u to page:%u\n", item_id, page_id);
  return mui_err_t::ok;
}

void MuiPlusPlus::menuStart(muiItemId page, muiItemId item){
  // switch to page, if error, then select first page by default
  if( goPageId(page, item) != mui_err_t::ok){
    currentPage = pages.begin();
    (*currentPage).itm_selected = false;
  }
}


mui_err_t MuiPlusPlus::goPageId(muiItemId page_id, muiItemId item_id){
  // printf("goPageId:%u,%u\n", page_id, item_id);
  auto p = _page_by_id(page_id);
  // check if no such page or page has no any items at all?
  if ( p == pages.end() || !(*p).items.size() ){
    return mui_err_t::id_err;
  }

  // unfocus/unselect and notify current item if it is defined
  if ( (currentPage != pages.end()) && ((*currentPage).currentItem != (*currentPage).items.end()) && (*(*currentPage).currentItem)->focused ){
    (*(*currentPage).currentItem)->focused = false;
    (*(*currentPage).currentItem)->selected = false;
    (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::unfocus));
  }

  currentPage = p;
  // invalidate current item iterator
  (*currentPage).currentItem = (*currentPage).items.end();

  // try to focus and select specified item on a page
  if ( item_id && (goItmId(item_id) == mui_err_t::ok) ) return mui_err_t::ok;
  // else target item is not specified (==0) or not found

  // Let's check if autoselect item is defined for a page, then focus and select it
  if ((*currentPage).autoSelect && (goItmId((*currentPage).autoSelect) == mui_err_t::ok) ){
    return mui_err_t::ok;
  }

  // Otherwise, let's try to find any first focusable item on a page
  _any_focusable_item_on_a_page_b();
  // if nothing found, well... this page is empty or only static items
  return mui_err_t::ok;
}


mui_err_t MuiPlusPlus::goPageLbl(const char* label){
  auto p = _page_by_label(label);
  if ( p != pages.cend() ){
    return goPageId((*p).id);
  }
  return mui_err_t::id_err;
}

mui_err_t MuiPlusPlus::goItmId(muiItemId item_id){
  if (!item_id) return mui_err_t::id_err;
  // if I need to switch to specific item on a page, let's check if it is registered there
  auto it = std::find_if( (*currentPage).items.begin(), (*currentPage).items.end(), muipp::MatchID<MuiItem_pt>(item_id) );
  if (it == (*currentPage).items.end()) return mui_err_t::id_err;

  // OK, item is indeed found, we are happy, check if it is not static
  if ( (*it)->getConstant() )
    return mui_err_t::id_err;

  // unfocus and notify current item if it is defined and focused
  if ( (currentPage != pages.end()) && ((*currentPage).currentItem != (*currentPage).items.end()) && (*(*currentPage).currentItem)->focused ){
    (*(*currentPage).currentItem)->focused = false;
    (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::unfocus));
  }

  (*currentPage).currentItem = it;
  // check if item is selectable, then focus on it and select it
  if ((*it)->getSelectable()){
    (*currentPage).itm_selected = true;
    (*(*currentPage).currentItem)->selected = true;
  }
  // update item's focus flag, we focus on it anyway, event if it' not selectable
  (*it)->focused = true;
  // notify item that it received focus
  (*it)->muiEvent(mui_event(mui_event_t::focus));
  return mui_err_t::ok;
}


void MuiPlusPlus::render(void* r){
  // won't run with no pages or items
  if (!pages.size() || !items.size())
    return;

  //// printf("Render %u items on page:%u\n", (*currentPage).items.size(), (*currentPage).id);

  // render each item on a page
  for (auto itm : (*currentPage).items ){
    //// printf("Render item:%u\n", id);
    // render selected item passing it a reference to current page
    (*itm).render(&(*currentPage), r);
  }
}

bool MuiPlusPlus::refresh(void* r){
  bool rr{false};
  // won't run with no pages or items
  if (!pages.size() || !items.size())
    return rr;

  //Serial.printf("Render %u items on page:%u\n", (*currentPage).items.size(), (*currentPage).id);

  // render each item on a page if needed
  for (auto itm : (*currentPage).items ){
    bool item_refresh = (*itm).refresh_req();
    if (item_refresh){
      // render selected item passing it a reference to current page
      (*itm).render(&(*currentPage), r);
      rr = true;
    }
  }
  return rr;
}

mui_event MuiPlusPlus::muiEvent(mui_event e){
  // printf("MPP event:%u\n", static_cast<uint32_t>(e.eid));
  _evt_recursion = 0;
  if (e.eid == mui_event_t::noop) return e;

  // if focused Item on current page exist and active - pass navigation and value events there and process reply event
  if ( (*currentPage).currentItem != (*currentPage).items.end() ){
    // if item is selected then it could receive cursor + value events, it's resone will be forwarded to _menu_navigation() call
    if ((*currentPage).itm_selected && (static_cast<size_t>(e.eid) < 100 || static_cast<size_t>(e.eid) >= 200) ){
      return _menu_navigation( (* (*currentPage).currentItem )->muiEvent(e) );
    }

    // if item is not selectable, then it can still receive value and "enter" events without grabbing cursor navigation events, it's resone will be forwarded to _menu_navigation() call
    if ((* (*currentPage).currentItem )->getSelectable() == false && (e.eid == mui_event_t::enter || static_cast<size_t>(e.eid) >= 200) ){
      return _menu_navigation( (* (*currentPage).currentItem )->muiEvent(e) );
    }
  }


  // otherwise pass event to menu navigation function
  if (static_cast<size_t>(e.eid) < 200)
    return _menu_navigation(e);

  return {};
}

mui_event MuiPlusPlus::_menu_navigation(mui_event e){
  // do not work on empty pages (for now), check recursion level
  if ( (*currentPage).items.size() == 0 || (++_evt_recursion > MAX_NESTED_EVENTS) ) return {};
  // printf("_menu_navigation evt:%u, recursion:%u\n", static_cast<uint32_t>(e.eid), _evt_recursion);

  switch(e.eid){
    // cursor actions

    // move focus to previous item on a page
    case mui_event_t::moveUp :
    case mui_event_t::moveLeft :
      _evt_prevItm();
      break;

    // move focus to next item on a page
    case mui_event_t::moveDown :
    case mui_event_t::moveRight :
      _evt_nextItm();
      break;

    // enter/action event
    case mui_event_t::enter :
      // if focused item is selectable, mark it as 'selected', it will start stealing cursor events from menu navigator untill released
      if ( currentPage == pages.end() || (*currentPage).currentItem == (*currentPage).items.end() || !(*(*currentPage).currentItem) ) break;    // if any of iterators are invalidated
      if ((*(*currentPage).currentItem)->getSelectable()){
        (*currentPage).itm_selected = true;
        (*(*currentPage).currentItem)->selected = true;
        // send "select" event to the item
        // maybe I need recursive call to self here?
        _menu_navigation( (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::select)) );
      }
      break;

    // go to previous page
    case mui_event_t::prevPage :
      return _prev_page();

    // go to page by name label
    case mui_event_t::goPageByName :
      goPageLbl( static_cast<const char*>(e.arg) );
      break;

    // go to page by id
    case mui_event_t::goPageByID :
      goPageId( static_cast<muiItemId>(e.param) );
      break;

    case mui_event_t::escape :
      return _evt_escape();

    case mui_event_t::quitMenu :
      return mui_event(mui_event_t::quitMenu);

  }

  // no-op
  return {};
}

void MuiPlusPlus::_feedback_event(mui_event e){
  // printf("_feedback_event:%u\n", static_cast<uint32_t>(e.eid));
  switch(e.eid){
    case mui_event_t::prevPage :
      _prev_page();
      break;
    case mui_event_t::goPageByName :
      goPageLbl(static_cast<const char*>(e.arg) );
      break;
    // 
  }
}

mui_event MuiPlusPlus::_prev_page(){
  // check if current page has any parent page 
  if ( (*currentPage).parent_page ){
    goPageId((*currentPage).parent_page);
    return {};
  }

  // otherwise I'm at the top level and all I can do is to signal menu quit
  return mui_event(mui_event_t::quitMenu);
}

mui_err_t MuiPlusPlus::pageAutoSelect(muiItemId page_id, muiItemId item_id){
  // printf("pageAutoSelect:%u,%u\n", page_id, item_id);
  auto p = _page_by_id(page_id);
  if ( p == pages.cend() ){
    return mui_err_t::id_err;
  }

  auto it = std::find_if( (*p).items.begin(), (*p).items.end(), muipp::MatchID<MuiItem_pt>(item_id) );
  if (it != (*p).items.end()){
    // OK, item is indeed found, we are happy
    (*p).autoSelect = item_id;
    return mui_err_t::ok;
  }

  return mui_err_t::id_err;
}

mui_event MuiPlusPlus::_evt_escape(){
  if (currentPage == pages.end()){
    // I'm in some undeterminated state where curent page does not exist, signal to quit the menu
    return mui_event(mui_event_t::quitMenu);
  }

  // first unselect current item if it's selected and let menu navigation work on moving focus on other items
  if ((*currentPage).itm_selected){
    (*currentPage).itm_selected = false;
    (*(*currentPage).currentItem)->selected = false;
    // notify item that it lost selection
    (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::unselect));
    return {};
  }

  // else I'm on a page and got escape event, I might try to switch to previos page,
  // it will either switch or signal to quit the menu
  return _prev_page();
}

uint32_t MuiPlusPlus::nextIndex(){
  do {
    ++_items_index;
  } while(_item_by_id(_items_index) != items.end());

  return _items_index;
}

mui_err_t MuiPlusPlus::_evt_nextItm(){
  // printf("_evt_nextItm\n");
  if ( !(*currentPage).items.size() || (*currentPage).currentItem == (*currentPage).items.end() ){
    // invalid iterator, nothing on page we can work on
    // printf("no valid items on a page!\n");
    return mui_err_t::id_err;
  }

  if (!(*(*currentPage).currentItem)->getConstant()){
    (*(*currentPage).currentItem)->focused = false;
    // notify current item that it has lost focus
    (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::unfocus));
  }
  // move focus on next item
  while ( ++(*currentPage).currentItem != (*currentPage).items.end() ){
    // stop on first non-const item
    if ( !(*(*currentPage).currentItem)->getConstant() ) break;
  }

//  do{
//    ++(*currentPage).currentItem;
//  } while( ((*currentPage).currentItem != (*currentPage).items.end()) || (*(*currentPage).currentItem)->getConstant() );

  if ((*currentPage).currentItem == (*currentPage).items.end())
    return _any_focusable_item_on_a_page_b();
  else {
    // update focus flag
    (*(*currentPage).currentItem)->focused = true;
    // notify item that it received focus
    _menu_navigation( (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::focus)) );
  }
  return mui_err_t::ok;
}

mui_err_t MuiPlusPlus::_evt_prevItm(){
  // printf("_evt_prevItm\n");
  if ( !(*currentPage).items.size() || (*currentPage).currentItem == (*currentPage).items.end() ){
    // invalid iterator, nothing on page we can work on
    // printf("no valid items on a page!\n");
    return mui_err_t::id_err;
  }

  if (!(*(*currentPage).currentItem)->getConstant()){
    (*(*currentPage).currentItem)->focused = false;
    // notify current item that it has lost focus
    (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::unfocus));
  }
  // move focus on prev item (if we are at first item, cycle to the last)
  if ((*currentPage).currentItem == (*currentPage).items.begin())
    (*currentPage).currentItem = std::prev( (*currentPage).items.end() );

  while ( --(*currentPage).currentItem != (*currentPage).items.begin()){
    // printf("p1\n");
    // stop on first non-const item
    if ( !(*(*currentPage).currentItem)->getConstant() ) break;
  }

//  do {
//    // printf("p1\n");
//    --(*currentPage).currentItem;
//  } while( (*currentPage).currentItem != (*currentPage).items.begin() || (*(*currentPage).currentItem)->getConstant() );
    // printf("p2\n");

  // check if last iterator is reached head of list and still we have constant element that we can't focus on
  if ((*currentPage).currentItem == (*currentPage).items.begin() && (*(*currentPage).currentItem)->getConstant())
    return _any_focusable_item_on_a_page_e();
  else {
    // printf("p4\n");
    // update focus flag
    (*(*currentPage).currentItem)->focused = true;
    // notify item that it received focus
    _menu_navigation( (*(*currentPage).currentItem)->muiEvent(mui_event(mui_event_t::focus)) );
  }
  return mui_err_t::ok;
}

mui_err_t MuiPlusPlus::_any_focusable_item_on_a_page_b(){
  // printf("_any_focusable_item_on_a_page_b\n");
  for (auto it = (*currentPage).items.begin(); it != (*currentPage).items.end(); ++it){
    if ( (*it)->getConstant() )
      continue;
    (*currentPage).currentItem = it;
    // update new item's focus flag
    (*it)->focused = true;
    // notify item that it received focus
    (*it)->muiEvent(mui_event(mui_event_t::focus));
    return mui_err_t::ok;
  }

  return mui_err_t::id_err;
}

mui_err_t MuiPlusPlus::_any_focusable_item_on_a_page_e(){
  // printf("_any_focusable_item_on_a_page_e\n");
  if ( !(*currentPage).items.size())   return mui_err_t::id_err;
  for (auto it = std::prev( (*currentPage).items.end() ); it != (*currentPage).items.begin(); --it){
    if ( (*it)->getConstant() )
      continue;
    (*currentPage).currentItem = it;
    // update new item's focus flag
    (*it)->focused = true;
    // notify item that it received focus
    (*it)->muiEvent(mui_event(mui_event_t::focus));
    return mui_err_t::ok;
  }

  // invalidate iterator
  (*currentPage).currentItem = (*currentPage).items.end();
  return mui_err_t::id_err;
}

void MuiPlusPlus::clear(){
  pages.clear();
  items.clear();
  _items_index = _pages_index = 0;
  currentPage = pages.end();
}

void MuiPlusPlus::removeItem(muiItemId item_id){
  // remove item from all the pages
  for (auto &i : pages){
    i.removeItem(item_id);
  }
  // erase the item itself
  // Replace std::erase_if with remove_if + erase
  items.erase(
    std::remove_if(items.begin(), items.end(), muipp::MatchID<MuiItem_pt>(item_id)),
    items.end()
  );
}
