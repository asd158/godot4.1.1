/**************************************************************************/
/*  lru.h                                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef LRU_H
#define LRU_H

#include "core/math/math_funcs.h"
#include "hash_map.h"
#include "list.h"

/**
LRU (Least Recently Used) 移除策略是 LruCache 中使用的一种缓存淘汰算法。
LRU 的全称是 Least Recently Used,也就是最近最少使用。它的基本思想是:
如果一个数据在最近一段时间没有被访问或使用,那么在将来它被访问的可能性也会低。
所以LRU算法会优先移除最不经常被访问的数据,也就是那些“最近最少使用”的数据,以此来为新的数据腾出空间。
具体来说,LruCache 中的数据是以链表来存储的。当一个数据被访问时,会将它移到链表头部,也就是说最新访问的数据在头部,久未访问的数据在尾部。当缓存满时需要移除数据,那么就可以直接移除链表尾部的数据,因为那是最不经常访问的数据。
这种算法实际上是利用了“最近使用的setData可能再次被访问,那么应该保留在缓存中;久未访问的数据可能不会被访问,那么优先从缓存中移除”这一局部性原理来设计的。
所以LRU算法能够保留热点数据,移除冷数据,从而使缓存命中率得到提升。这就是 LruCache 选择 LRU作为缓存置换算法的原因。
 */
template <class TKey, class TData, class Hasher = HashMapHasherDefault, class Comparator = HashMapComparatorDefault<TKey>>
class LRUCache {
private:
	struct Pair {
		TKey key;
		TData data;

		Pair() {}
		Pair(const TKey &p_key, const TData &p_data) :
				key(p_key),
				data(p_data) {
		}
	};

	typedef typename List<Pair>::Element *Element;

	List<Pair> _list;
	HashMap<TKey, Element, Hasher, Comparator> _map;
	size_t capacity;

public:
	const TData *insert(const TKey &p_key, const TData &p_value) {
		Element *e = _map.getptr(p_key);
		Element n = _list.push_front(Pair(p_key, p_value));

		if (e) {
			_list.erase(*e);
			_map.erase(p_key);
		}
		_map[p_key] = _list.front();
        //擦除末尾的低利用率数据
		while (_map.size() > capacity) {
			Element d = _list.back();
			_map.erase(d->get().key);
			_list.pop_back();
		}

		return &n->get().data;
	}

	void clear() {
		_map.clear();
		_list.clear();
	}

	bool has(const TKey &p_key) const {
		return _map.getptr(p_key);
	}

	const TData &get(const TKey &p_key) {
		Element *e = _map.getptr(p_key);
		CRASH_COND(!e);
		_list.move_to_front(*e);
		return (*e)->get().data;
	};

	const TData *getptr(const TKey &p_key) {
		Element *e = _map.getptr(p_key);
		if (!e) {
			return nullptr;
		} else {
			_list.move_to_front(*e);
			return &(*e)->get().data;
		}
	}

	_FORCE_INLINE_ size_t get_capacity() const { return capacity; }
	_FORCE_INLINE_ size_t get_size() const { return _map.size(); }

	void set_capacity(size_t p_capacity) {
		if (capacity > 0) {
			capacity = p_capacity;
			while (_map.size() > capacity) {
				Element d = _list.back();
				_map.erase(d->get().key);
				_list.pop_back();
			}
		}
	}

	LRUCache() {
		capacity = 64;
	}

	LRUCache(int p_capacity) {
		capacity = p_capacity;
	}
};

#endif // LRU_H
