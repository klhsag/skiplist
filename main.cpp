#include<cstdlib>
#include<cstdio>

#include<ctime>

#include "skiplist.h"

using namespace mystd;

int main() {
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
		printf("not found.");
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
	system("pause");
	return 0;
}