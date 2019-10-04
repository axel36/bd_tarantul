#include "SimpleLRU.h"
#include <memory>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    if ( key.size() + value.size() > _max_size) {return false;}

    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {

        _insert_new_value(key, value);
    }else{

        _update_value(it->second.get(),value);
    }
    return true;
}

// See MacpBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    if ( key.size() + value.size() > _max_size) {return false;}

    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        _insert_new_value(key, value);
        return true;
    }
    else
        return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    if(value.size()+key.size() > _max_size ){return false;}

    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        _update_value(it->second.get(), value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) { return false; }
    _remove_node(it->second.get());
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) { return false;}
    value = it->second.get().value;
    _move_to_fresh_data(it->second.get());

    return true;
}

/*
 *          --+----+--   prev   --+----+--          --+----+--              --+----+--
 * nullptr<---|    |    <---------|    |    <---------|    |  <---------------|new |
 *            |data|              |data|              |data|                  |node|      (fresh data goes here)
 *            |    |  --------->  |    | --------->   |    | ------>nullptr   |    |
 *          --+----+--  next*s  --+----+--          --+----+--              --+----+--
 *              ^                                       ^
 *              |                                       |
 *       head_smart(data for delete)                 tail(freshest)
 *
 */
void SimpleLRU::_insert_new_value(const std::string &key, const std::string &value)
{
//    auto *new_node = new lru_node(key,value);

    while(_current_size > _max_size - key.size() - value.size()){
        _remove_node(*_lru_head);
    }

    auto new_node = std::make_unique<lru_node> (key,value);


    if( _lru_tail != nullptr)
    {
        new_node->prev = _lru_tail;
        _lru_tail->next.swap(new_node);
        _lru_tail = _lru_tail->next.get();

    } else{
        _lru_tail = new_node.get();
        _lru_head.swap(new_node);
    }

    _lru_index.insert(std::make_pair(key,std::reference_wrapper<lru_node>(*_lru_tail)));
    _current_size+=key.size() + value.size();

}

void SimpleLRU::_update_value(lru_node &update_node, const std::string &new_value) {

    size_t delta_size = new_value.size() - update_node.value.size();

    while (_current_size + delta_size > _max_size){
        _remove_node(*_lru_head);
    }

    update_node.value = new_value;
    _move_to_fresh_data(update_node);

    _current_size += delta_size;

}

void SimpleLRU::_remove_node(lru_node &node_to_delete){

    _lru_index.erase(node_to_delete.key);
    _current_size -= (_lru_tail->key).size() + (_lru_tail->value).size();

    std::unique_ptr<lru_node > tmp;
    if(&node_to_delete == _lru_head.get()){

        tmp.swap(_lru_head);
        _lru_head.swap(tmp->next);
        _lru_head->prev = nullptr;
        return;
    }
    if(&node_to_delete == _lru_tail) {
        tmp.swap(_lru_tail->prev->next);
        _lru_tail = tmp->prev;
        _lru_tail->next.reset(nullptr);
        return;
    }
    tmp.swap(node_to_delete.prev->next);
    tmp->prev->next.swap(tmp->next);
    tmp->next->prev = tmp->prev;

}

void SimpleLRU::_move_to_fresh_data(lru_node &recently_used)
{
    if (&recently_used == _lru_tail) { return;}

    if (&recently_used == _lru_head.get()){
        _lru_head.swap(recently_used.next);
        _lru_head->prev = nullptr;
    } else {
        recently_used.next->prev = recently_used.prev;
        recently_used.prev->next.swap(recently_used.next);
    }
    _lru_tail->next.swap(recently_used.next);
    recently_used.prev = _lru_tail;
    _lru_tail = &recently_used;
}

} // namespace Backend
} // namespace Afina
