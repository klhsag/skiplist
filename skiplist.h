#pragma once

#include<stdexcept>

#include<memory>

#include<utility>
#include<list>
#include<vector>

namespace mystd {

const unsigned int MAX_LEVEL = 30;

class skiplist_exception : std::exception {
public:
	skiplist_exception() = default;
	skiplist_exception(std::string info) : std::exception(info.c_str()) {}
};

template<typename T_key, typename T_val>
struct skiplist_data {
	std::pair<T_key, T_val> entry;
	skiplist_data(T_key key, T_val val) : entry({ key, val }) {}
	const T_key & key() const { return entry.first; }
	const T_val & val() const { return entry.second; }
};

template<typename T_key, typename T_val>
struct skiplist_node {
	std::unique_ptr< skiplist_data<T_key, T_val> > data;
	std::vector< std::shared_ptr< skiplist_node<T_key, T_val> > > nexts;
	std::vector< std::weak_ptr< skiplist_node<T_key, T_val> > > preds;         // For `head` , `preds` is useless.

	skiplist_node() : data(nullptr), nexts(), preds() {}
	skiplist_node(T_key key, T_val val) :
		data(std::make_unique< skiplist_data<T_key, T_val> >(key, val)),
		nexts(),
		preds() {}
	~skiplist_node() = default;

	std::pair<T_key, T_val> & entry() const { if (data) return data->entry; else throw skiplist_exception("void entry"); }
	const T_key & key() const { if (data) return data->key(); else throw skiplist_exception("void entry"); }
	const T_val & val() const { if (data) return data->val(); else throw skiplist_exception("void entry"); }
	std::shared_ptr< skiplist_node<T_key, T_val> > iter_by(unsigned int level) const { return nexts.at(level); }
	std::shared_ptr< skiplist_node<T_key, T_val> > iter_next() const { return iter_by(0); }
};

template<typename T_key, typename T_val>
class skiplist {

private:
	size_t size_;
	std::shared_ptr< skiplist_node<T_key, T_val> > head;

	/**
	*	Recursive Function
	*		p=head to start:
	*			Return the entry has a largest key which <= the given key. When no such key in skiplist, return `head`.
	*/
	std::shared_ptr< skiplist_node<T_key, T_val> > find_from(T_key key, std::shared_ptr< skiplist_node<T_key, T_val> > p) const {
		auto & ptrs = p->nexts;
		for (auto it = ptrs.rbegin(); it != ptrs.rend(); ++it) {
			auto q = *it;
			if (!q) continue;
			if (q->key() == key) return q;
			if (q->key() < key) return find_from(key, q);
		}
		return p;
	}

	/*	Recursive Function */
	void insert_from(T_key key, T_val val,
			unsigned int level, std::shared_ptr< skiplist_node<T_key, T_val> > new_node_ptr,
			std::shared_ptr< skiplist_node<T_key, T_val> > p) {
		auto & ptrs = p->nexts;
		for (auto it = ptrs.rbegin(); it != ptrs.rend(); ++it) {
			auto q = *it;
			if (!q) continue;
			if (q->key() == key) throw skiplist_exception("key already exists.");
			if (q->key() < key) {
				insert_from(key, val, level, new_node_ptr, q);
				break;
			}
		}
		try { // To improve!
			for (int i = 0; i < level; ++i) {
				auto q = p->nexts.at(i);                     // catch `std::out_of_range` -> break
				if ((!q) || (q->key() > key)) {
					p->nexts[i] = new_node_ptr;
					new_node_ptr->preds.push_back(p);
					new_node_ptr->nexts.push_back(q);
					if (q) q->preds[i] = new_node_ptr;
				}
			}
		} catch (std::out_of_range) {}
	}

	/* Delete the given node, which must exists.*/
	void delete_node(std::shared_ptr< skiplist_node<T_key, T_val> > target) {
		int l = target->nexts.size();
		// assert l == p->preds.size()
		for (int i = 0; i < l; ++i) {
			auto p = target->preds[i].lock();
			auto q = target->nexts[i];
			p->nexts[i] = q;
			if (q) q->preds[i] = p;
		}
	}
	
public:
	size_t size() const { return size_; }
	skiplist() : size_(0), head(std::make_shared< skiplist_node<T_key, T_val> >()) {}
	~skiplist() = default;

	const std::pair<T_key, T_val> & find(T_key key) const {
		auto candidate_ptr = find_from(key, head);  // assert _ != null
		try {
			if (candidate_ptr->key() == key) return candidate_ptr->entry();
			throw skiplist_exception("not found");
		} catch (skiplist_exception) {
			throw skiplist_exception("not found");
		}
	}
	void insert(T_key key, T_val val) {
		auto new_node_ptr = std::make_shared< skiplist_node<T_key, T_val> >(key, val);
		int level = 1;
		while (rand() % 32 < 15 && level < MAX_LEVEL) ++level;
		while (head->nexts.size() < level) head->nexts.push_back(nullptr);
		insert_from(key, val, level, new_node_ptr, head);
		size_ += 1;
	}
	void erase(T_key key) {
		auto candidate_ptr = find_from(key, head);
		try {
			if (candidate_ptr->key() == key) {
				delete_node(candidate_ptr);
			} else {
				throw skiplist_exception("not found");
			}
		} catch (skiplist_exception) {
			throw skiplist_exception("not found");
		}
		size_ -= 1;
	}
	std::vector< std::pair< T_key, T_val> > to_vector(int level = 0) const {
		std::vector< std::pair< T_key, T_val> > res;
		auto p = head;
		if (p) p = p->iter_by(level);
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
