#include <ilcplex/ilocplex.h>

using namespace std;

int verbose = 1;

class Boxes {

protected: 

	IloEnv env;
	IloModel model;

	IloIntVarArray x, y;
	IloBoolVarArray rot;
	IloIntVar L;

	int W;
	int num_boxes = 0;
	int max_L = 0;
	IloIntArray w, h;

public:

	Boxes()
	{
		env = IloEnv();
		model = IloModel(env);
		w = IloIntArray(env);
		h = IloIntArray(env);
	}

	~Boxes()
	{
		env.end();
	}

	void read_input()
	{
		cin >> W;

		int n, lw, lh;
		int min_area = 0;

		while (cin >> n >> lw >> lh) {
			num_boxes += n;
			for(int i = 0; i < n; i++) {
				max_L += lh;
				min_area += lw*lh;
				w.add(lw);
				h.add(lh);
			}
		}

		int min_L = (int) ceil(((double) min_area) / W);

		//if (verbose)
		//	fprintf(stderr, "W = %d, L in [%d, %d], num_boxes = %d\n", 
		//		W, min_L, max_L, num_boxes);

		//for(int i = 0; i < num_boxes; i++) {
		//	printf("%d %d\n", w[i], h[i]);
		//}

		//L = IntVar(*this, min_L, max_L);
		//x = IntVarArray(*this, num_boxes, 0, W);
		//y = IntVarArray(*this, num_boxes, 0, max_L);
		//rot = BoolVarArray(*this, num_boxes, 0, 1);

		L = IloIntVar(env, min_L, max_L);
		x = IloIntVarArray(env, num_boxes, 0, W);
		y = IloIntVarArray(env, num_boxes, 0, max_L);
		rot = IloBoolVarArray(env, num_boxes);

		for(int i = 0; i < num_boxes; i++)
		{
			if(w[i] == h[i]) model.add(rot[i] == 0);
			//else
			//	model.add(rot[i] >= 0);

			IloExpr wi = (1 - rot[i]) * w[i] + rot[i] * h[i];
			IloExpr hi = (1 - rot[i]) * h[i] + rot[i] * w[i];

			model.add(W >= x[i] + wi);
			model.add(L >= y[i] + hi);
		}

		for(int i = 0; i < num_boxes; i++) {
			for(int j = 0; j < num_boxes; j++) {
				if(i == j) continue;

				/* Similar boxes don't need to be tested again */
				if((i < j) && (w[i] == w[j]) && (h[i] == h[j]))
				{
					IloExpr pi = x[i] + y[i]*W;
					IloExpr pj = x[j] + y[j]*W;

					model.add(pi <= pj);
				}

				IloExpr wi = (1 - rot[i]) * w[i] + rot[i] * h[i];
				IloExpr hi = (1 - rot[i]) * h[i] + rot[i] * w[i];
				IloExpr wj = (1 - rot[j]) * w[j] + rot[j] * h[j];
				IloExpr hj = (1 - rot[j]) * h[j] + rot[j] * w[j];

				model.add(
						// x axis
						x[i] + wi <= x[j] || 
						x[j] + wj <= x[i] ||
						// y axis
						y[i] + hi <= y[j] ||
						y[j] + hj <= y[i]);
			}
		}

		model.add(IloMinimize(env, L));
	}

	void solve()
	{
		IloCplex cplex(model);
		//cplex.setOut(env.getNullStream());
		cplex.setOut(cerr);
		cplex.setParam(IloCplex::TiLim, 115);
		cplex.solve();

		cout << cplex.getObjValue() << endl;

		IloNumArray xx(env), yy(env);
		IloNumArray rotv(env);
		cplex.getValues(xx, x);
		cplex.getValues(yy, y);
		cplex.getValues(rotv, rot);

		for(int i = 0; i < num_boxes; i++)
		{
			int wi = w[i], hi = h[i];
			if (rotv[i])
			{
				wi = h[i];
				hi = w[i];
			}

			cout << xx[i] << " " << yy[i] << "    " << xx[i] + wi - 1 << " " << yy[i] + hi - 1 << endl;
		}

	}

};

int main (int argc, char *argv[])
{
	Boxes boxes;

	boxes.read_input();
	boxes.solve();

	//x.add(IloNumVar(env, 0, 40));
	//x.add(IloNumVar(env));
	//x.add(IloNumVar(env));
	//x.add(IloNumVar(env, 2, 3, ILOINT));

	//model.add( - x[0] + x[1] + x[2] + 10 * x[3] <= 20);
	//model.add( x[0] - 3 * x[1] + x[2] <= 30);
	//model.add( x[1] - 3.5* x[3] ==  0);

	//model.add(IloMaximize(env, x[0]+2*x[1]+3*x[2]+x[3]));

	//IloCplex cplex(model);
	//cplex.solve();

	//cout << "Max=" << cplex.getObjValue() << endl;

	//env.end();

	return 0;
}
