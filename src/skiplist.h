#pragma once

#include<stdexcept>

#include<memory>

#include<utility>
#include<list>
#include<vector>

namespace mystd {

const int MAX_LEVEL = 30;

size_t CREATE_LEVEL() {
	int level = 1;
	while (rand() % 32 < 15 && level < MAX_LEVEL) ++level;
	return level;
}

class skiplist_exception : public std::runtime_error {
public:
	skiplist_exception() = default;
	skiplist_exception(std::string info) : std::runtime_error(info.c_str()) {}
};

template<typename T_key, typename T_val>
struct skiplist_data {
	std::pair<T_key, T_val> entry;
	skiplist_data(T_key key, T_val val) : entry({ key, val }) {}
	const T_key & key() const { return entry.first; }
	const T_val & val() const { return entry.second; }
};

template<typename T_key, typename T_val>
struct skiplist_node /*: public std::enable_shared_from_this<skiplist_node<T_key, T_val> >*/ {
	std::unique_ptr< skiplist_data<T_key, T_val> > data;
	std::vector< size_t > sizes;
	std::vector< std::shared_ptr< skiplist_node<T_key, T_val> > > nexts;
	std::vector< std::weak_ptr< skiplist_node<T_key, T_val> > > preds;

	skiplist_node(T_key key, T_val val, size_t level) :
		data(std::make_unique< skiplist_data<T_key, T_val> >(key, val)),
		sizes(level, 1),
		nexts(level),
		preds(level) {}
	skiplist_node(std::unique_ptr< skiplist_data<T_key, T_val> > data, size_t level):
		data(std::move(data)),
		sizes(level, 1),
		nexts(level),
		preds(level) {}
	~skiplist_node() = default;

	size_t level() const { return nexts.size(); }
	size_t size() const { return sizes[level()-1]; }
	std::pair<T_key, T_val> & entry() const { if (data) return data->entry; else throw skiplist_exception("void entry"); }
	const T_key & key() const { if (data) return data->key(); else throw skiplist_exception("void entry"); }
	const T_val & val() const { if (data) return data->val(); else throw skiplist_exception("void entry"); }
	std::shared_ptr< skiplist_node<T_key, T_val> > iter_by(unsigned int level) const { return nexts.at(level); }
	std::shared_ptr< skiplist_node<T_key, T_val> > iter_next() const { return iter_by(0); }
};

/* find the largest node with key <= given key */
template<typename T_key, typename T_val>
std::shared_ptr< skiplist_node<T_key, T_val> > find_skiplist_node(std::shared_ptr< skiplist_node<T_key, T_val> > head, int level, T_key key) {
	if (level < 0 || head->data->key() == key) return head;
	if (head->nexts[level] && head->nexts[level]->key() <= key) return find_skiplist_node(head->nexts[level], level, key);
	return find_skiplist_node(head, level - 1, key);
}

/* insert data with spec~ level, which no larger than head's level */
template<typename T_key, typename T_val>
void insert_skiplist_node(std::shared_ptr< skiplist_node<T_key, T_val> > head, int level, std::unique_ptr< skiplist_data<T_key, T_val> > data) {
	T_key key = data->key();
	auto p = find_skiplist_node(head, head->level() - 1, key);
	if (p->data->key() == key) throw skiplist_exception("duplicate key");
	auto target = std::make_shared< skiplist_node<T_key, T_val> >(std::move(data), level);
	size_t cnt = 1;
	for (size_t i = 0; i<level; ++i) {
		// assert : head already has the max level
		while (p->level() <= i) {
			p = p->preds[p->level() - 1].lock();
			cnt += p->sizes[p->level() - 1];
		}
		size_t new_size = cnt;
		size_t old_size = p->sizes[i];
		size_t tgt_size = old_size - new_size + 1;
		auto q = p->nexts[i];
		p->nexts[i] = target;
		target->preds[i] = p;
		target->nexts[i] = q;
		if (q) q->preds[i] = target;
		p->sizes[i] = new_size;
		target->sizes[i] = tgt_size;
	}

	for (size_t i = level; i<head->level(); ++i) {
		while (p->level() <= i) {
			p = p->preds[p->level() - 1].lock();
		}
		++p->sizes[i];
	}

}

/* delete a non-head node from skiplist and return true, or retrun false when key not exist */
template<typename T_key, typename T_val>
bool delete_skiplist_node(std::shared_ptr< skiplist_node<T_key, T_val> > head, T_key key) {
	auto target = find_skiplist_node(head, head->level() - 1, key);
	if (target->data->key() != key) return false;
	size_t level = target->level();
	for (size_t i = 0; i<level; ++i) {
		// assert : never delete head
		auto p = target->preds[i].lock();
		auto q = target->nexts[i];
		p->nexts[i] = q;
		if (q) q->preds[i] = p;
		p->sizes[i] = p->sizes[i] + target->sizes[i] - 1;
	}
	return true;
}

template<typename T_key, typename T_val>
std::shared_ptr< skiplist_node<T_key, T_val> > idx_skiplist_node(std::shared_ptr< skiplist_node<T_key, T_val> > head, int level, size_t idx) {
	// assert idx >= 0
	if (level < 0) return head;
	size_t local_size = head->sizes[level];
	if (idx < local_size) return idx_skiplist_node(head, level - 1, idx);
	if (!head->nexts[level]) throw std::out_of_range("invaild idx");
	return idx_skiplist_node(head->nexts[level], level, idx - local_size);
}

/*template<typename T_key, typename T_val>
class skiplist_iterator {
private:
	std::shared_ptr< skiplist_node<T_key, T_val> > item;
public:
	skiplist_iterator(std::shared_ptr< skiplist_node<T_key, T_val> > node) : item(node) {}
	~skiplist_iterator() = default;

	std::pair<T_key, T_val> & operator*() const {
		return item->data->entry();
	}
	skiplist_iterator<T_key, T_val> operator++() {
		item = item->iter_next();
	}

};*/

template<typename T_key, typename T_val>
class skiplist {

private:
	std::shared_ptr< skiplist_node<T_key, T_val> > head;

	std::shared_ptr< skiplist_node<T_key, T_val> > get_by_idx(size_t idx) const {
		if (idx < 0 || idx >= size()) throw std::out_of_range("invaild idx");
		auto res = idx_skiplist_node(head, head->level() - 1, idx);
		return res;
	}
	void insert_data(std::unique_ptr< skiplist_data<T_key, T_val> > data) {
		if (!head) {
			head = std::make_shared< skiplist_node<T_key, T_val> >(std::move(data), 1);
		} else {
			if (data->key() < head->data->key()) std::swap(data, head->data);
			size_t new_level = CREATE_LEVEL();
			while (head->level() <= new_level) {   // keepping head's top ptr is null so that size() is O(1)
				head->nexts.push_back(nullptr);
				head->sizes.push_back(*(head->sizes.rbegin()));
				head->preds.push_back(std::shared_ptr< skiplist_node<T_key, T_val> >(nullptr));
			}
			insert_skiplist_node(head, new_level, std::move(data));
		}
	}
	bool find_and_erase(T_key key) {
		if (key == head->data->key()) {
			if (head->size() == 1) {
				head = nullptr;
				return true;
			} else {
				auto next_item = head->iter_next();
				std::swap(head->data, next_item->data);
			}
		}
		return delete_skiplist_node(head, key);
	}
	
public:
	size_t size() const { return head ? head->size() : 0; }
	skiplist() = default;
	~skiplist() = default;

	std::shared_ptr< skiplist_node<T_key, T_val> > find_upper_bound(T_key key) const {
		return find_skiplist_node(head, head->level() - 1, key);
	}

	const std::pair<T_key, T_val> & at(size_t idx) const {
		auto node = get_by_idx(idx);
		return node->entry();
	}
	const std::pair<T_key, T_val> & find(T_key key) const {
		auto candidate = find_upper_bound(key);  // assert _ != null
		if (candidate->key() == key) return candidate->entry();
		throw skiplist_exception("not found");
	}
	void insert(T_key key, T_val val) {
		auto data = std::make_unique< skiplist_data<T_key, T_val> >(key, val);
		insert_data(std::move(data));
	}
	void erase(T_key key) {
		if (!find_and_erase(key)) throw skiplist_exception("not found");
	}
	std::vector< std::pair< T_key, T_val> > to_vector(int level = 0) const {
		std::vector< std::pair< T_key, T_val> > res;
		auto p = head;
		while (p) {
			res.push_back(p->entry());
			p = p->iter_by(level);
		}
		return res;
	}
	std::vector< std::vector< std::pair< T_key, T_val> > > show_2d() const {
		std::vector< std::vector< std::pair< T_key, T_val> > > res;
		int l = head->nexts.size();
		for (int i = l - 1; i >= 0; --i) {
			res.push_back(to_vector(i));
		}
		return res;
	}

};

}
