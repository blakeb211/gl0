#include <cstdio>
#include <vector>

using std::vector;

struct v3 {
	float x,y,z;
};

struct entity {
	v3 pos;
};

struct grid {
	v3 center;
	v3 dp;
	grid* children[8];
	vector<entity*> ent;
	void insert(entity * e) {


	}
	void update(entity * e, v3 pos) {

	}
	vector<entity*> get_neighbors(entity *) {


	}
};

