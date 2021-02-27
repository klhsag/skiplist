#define _CRT_SECURE_NO_WARNINGS

#include<cstdlib>
#include<cstdio>

#include<ctime>

#include<algorithm>

#include "skiplist.h"

using namespace mystd;

int main() {   // https://ac.nowcoder.com/acm/problem/20026

	int n, a;
	scanf("%d", &n);

	skiplist<int, int> sklst;

	scanf("%d", &a);
	sklst.insert(a, 0);
	int res = a;
	for (int i = 1; i < n; ++i) {
		scanf("%d", &a);
		auto node1 = sklst.find_upper_bound(a);
		auto node2 = node1->iter_next();
		if (node2) res += std::min(abs(node1->data->key() - a), abs(node2->data->key() - a));
		else res += abs(node1->data->key() - a);
		try {
			sklst.insert(a, 0);
		}
		catch (...) {}
	}

	printf("%d\n", res);
	system("pause");

	return 0;
}



int test() {
	srand(time(NULL));
	skiplist<int, int> sklst;
	sklst.insert(3, 7);
	sklst.insert(8, 3);
	sklst.insert(5, 7);
	sklst.insert(20, 11);
	sklst.insert(7, 3);
	sklst.insert(15, 6);
	sklst.insert(26, 3);
	sklst.insert(212, 16);
	sklst.insert(4, 9);
	sklst.insert(-5, 3);
	sklst.insert(77, 5);
	auto v = sklst.to_vector();
	for (auto & p : v) {
		printf("-<%d %d>-", p.first, p.second);
	}
	printf("\n");
	printf("=================================\n");
	auto m = sklst.show_2d();
	for (auto & line : m) {
		printf("%d))", line.size());
		for (auto & p : line) {
			printf("-<%d %d>-", p.first, p.second);
		}
		printf("\n");
	}
	printf("=================================\n");
	try{
		sklst.erase(8);
		sklst.erase(212);
	} catch (mystd::skiplist_exception e) {
		printf("not found.\n");
	}
	m = sklst.show_2d();
	for (auto & line : m) {
		printf("%d))", line.size());
		for (auto & p : line) {
			printf("-<%d %d>-", p.first, p.second);
		}
		printf("\n");
	}

	try {
		printf(":-<%d %d>-", 3, sklst.find(3).second);
	} catch (mystd::skiplist_exception e) {
		printf("not found.");
	}
	{
		printf(":-<%d %d>-", sklst.at(4).first, sklst.at(4).second);
	}

	system("pause");
	return 0;
}