#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace Gecode;
using namespace std;

class Roll : public Space {
protected:
	
	IntVarArray x, y;
	BoolVarArray rot;
	int W;
	int num_boxes = 0;
	int max_L = 0;
	vector<int> w, h;
public:
	IntVar L; // Paper length

	Roll(Roll& s) : Space(s) {
		//printf("%x \n", &L);
		L.update(*this, s.L);
		x.update(*this, s.x);
		y.update(*this, s.y);
		rot.update(*this, s.rot);
		W = s.W;
		num_boxes = s.num_boxes;
		max_L = s.max_L;
		w = s.w;
		h = s.h;
	}
	Roll(int policy) {
		cin >> W;

		int n, lw, lh;
		int min_area = 0;

		while (cin >> n >> lw >> lh) {
			num_boxes += n;
			for(int i = 0; i < n; i++) {
				max_L += lh;
				min_area += lw*lh;
				w.push_back(lw);
				h.push_back(lh);
			}
		}

		int min_L = (int) ceil(((double) min_area) / W);

		fprintf(stderr, "W = %d, L in [%d, %d], num_boxes = %d\n", 
				W, min_L, max_L, num_boxes);

		//for(int i = 0; i < num_boxes; i++) {
		//	printf("%d %d\n", w[i], h[i]);
		//}

		L = IntVar(*this, min_L, max_L);
		x = IntVarArray(*this, num_boxes, 0, W);
		y = IntVarArray(*this, num_boxes, 0, max_L);
		rot = BoolVarArray(*this, num_boxes, 0, 1);

		for(int i = 0; i < num_boxes; i++) {
			rel(*this, (rot[i] && (L >= y[i] + w[i])) || (!rot[i] && (L >= y[i] + h[i])));
			rel(*this, (rot[i] && (W >= x[i] + h[i])) || (!rot[i] && (W >= x[i] + w[i])));
			if(w[i] == h[i]) rel(*this, rot[i] == 0);
		}

		//rel(*this, x, IRT_GE, 0);
		//rel(*this, y, IRT_GE, 0);
		//printf("OK\n");
		//cout << y[0] << endl;
		//rel(*this, y >= 0);
		// FIXME nooverlap doesn't rotate the boxes
		//nooverlap(*this, x, w, y, h);
		//nooverlap(*this, x, h, y, w, rot);
		for(int i = 0; i < num_boxes; i++) {
			for(int j = 0; j < num_boxes; j++) {
				if(i == j) continue;
				rel(*this,
						(!rot[i] && (x[i] + w[i] <= x[j])) ||
						(!rot[j] && (x[j] + w[j] <= x[i])) ||
						(!rot[i] && (y[i] + h[i] <= y[j])) ||
						(!rot[j] && (y[j] + h[j] <= y[i])) ||

						( rot[i] && (x[i] + h[i] <= x[j])) ||
						( rot[j] && (x[j] + h[j] <= x[i])) ||
						( rot[i] && (y[i] + w[i] <= y[j])) ||
						( rot[j] && (y[j] + w[j] <= y[i]))
				);
			}
		}

		Rnd r(1U);

		//branch(*this, rot, BOOL_VAR_RND(r), BOOL_VAL_RND(r));
		
		branch(*this, rot, BOOL_VAR_RND(r), BOOL_VAL_MIN());




		
		switch (policy) {
			case 1:
				// GOOD
				branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));
				branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));
				break;
			default:
				// MEH
				branch(*this, x, INT_VAR_RND(r), INT_VAL_RND(r));
				branch(*this, y, INT_VAR_RND(r), INT_VAL_RND(r));
				break;
		}



		// BAD
		//branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_RANGE_MIN());
		//branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_RANGE_MIN());
		
		// BAD
		//branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_RANGE_MAX());
		//branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_RANGE_MAX());

		// BAD
		//branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));
		//branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));

		// VERY BAD
		//branch(*this, x, INT_VAR_RND(r), INT_VAL_RANGE_MIN());
		//branch(*this, y, INT_VAR_RND(r), INT_VAL_RANGE_MIN());

		// Same as x and y
		//branch(*this, x+y, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));

		// MEH
		//branch(*this, x+y, INT_VAR_RND(r), INT_VAL_RND(r));




		//branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));
		//branch(*this, x, INT_VAR_RND(r), INT_VAL_MIN());
		//branch(*this, x+y, INT_VAR_RND(r), INT_VAL_RND(r));
		//branch(*this, y, INT_VAR_RND(r), INT_VAL_MIN());
		//branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));
		//branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		//branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_RND(r));
		//branch(*this, x+y, INT_VAR_RND(r), INT_VAL_RND(r));
		branch(*this, L, INT_VAL_MIN());
		//branch(*this, y, INT_VAR_RND(r), INT_VAL_RND(r));
		//branch(*this, rot, BOOL_VAR_RND(r), BOOL_VAL_RND(r));

	}

	virtual void constrain(const Space& _b) {
		const Roll& b = static_cast<const Roll&>(_b);
		rel(*this, L < b.L.val());
	}

	virtual Space* copy(bool share) {
		return new Roll(*this);
	}

	virtual Space* copy() {
		return new Roll(*this);
	}

	void print(void) const {
		cout << W << " " << L << endl;
		for(int i = 0; i < num_boxes; i++) {
			if (rot[i].val()) {
				// Mark rotated boxes by indenting them
				cout << "\t\t";
				cout << x[i] << " " << y[i];
				cout << "\t";
				cout << x[i].val() + h[i] - 1 << " " << y[i].val() + w[i] - 1 << endl;
			} else {
				cout << x[i] << " " << y[i];
				cout << "\t";
				cout << x[i].val() + w[i] - 1 << " " << y[i].val() + h[i] - 1 << endl;
			}
		}
	}
	void status(void) const {
		fprintf(stderr, "Best solution: L = %d\n", L.val());
	}
};

int main(int argc, char* argv[]) {

	int time = 120; // 120 seconds by default
	int policy = 1;
	clock_t tic, toc;
	double dt;

	if (argc >= 2)
		time = atoi(argv[1]);
	if (argc >= 3)
		policy = atoi(argv[2]);

	fprintf(stderr, "Time limit set to %d seconds\n", time);
	fprintf(stderr, "Policy set to %d\n", policy);

	Roll* m = new Roll(policy);
	Search::Stop* stop = new Search::TimeStop(time * 1000);
	Search::Options* o = new Search::Options();
	o->stop = stop;
	o->threads = 2;

	tic = clock();

	//DFS<Roll> e(m);
	
	BAB<Roll> e(m, *o);
	//delete m;
	Roll *best = m;
	while (Roll *s = e.next()) {
		best = s;
		//best->status();
		toc = clock();
		dt = ((double) toc - tic) / CLOCKS_PER_SEC;
		fprintf(stderr, "L = %d\tt = %.3f s\n", s->L.val(), dt);
		//delete s;
		//break;
	}
	Search::Statistics stats = e.statistics();
	fprintf(stderr, "Nodes explored:   %ld\n", stats.node);
	fprintf(stderr, "Failed nodes:     %ld\n", stats.fail);
	fprintf(stderr, "Propagators used: %ld\n", stats.propagate);
	fprintf(stderr, "Depth reached:    %ld\n", stats.depth);
	best->print();
	return 0;
}