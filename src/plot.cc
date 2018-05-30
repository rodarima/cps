#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
	int L, x0, y0, x1, y1, W=0, num_boxes=0;
	vector <int> x,y,X,Y;

	cin >> W;
	cin >> L;

	while (cin >> x0 >> y0 >> x1 >> y1){
		num_boxes++;
		x.push_back(x0);
		y.push_back(y0);
		X.push_back(x1);
		Y.push_back(y1);
	}

	printf("Plotting a roll of W = %d and L = %d with %d boxes.\n", W, L, num_boxes);

	vector<vector<int>> screen;

	for(int i=0; i<L; i++) {
		vector<int> row(W, 0);
		screen.push_back(row);
	}

	for(int i=0; i<num_boxes; i++) {
		x0 = x[i];
		y0 = y[i];
		x1 = X[i];
		y1 = Y[i];
		
		for (int xx=x0; xx<=x1; xx++)
		{
			for (int yy=y0; yy<=y1; yy++)
			{
				int val = screen[yy][xx];
				if (val != 0)
					screen[yy][xx] = -1;
				else 
					screen[yy][xx] = i+1;
			}
		}

		/*
		// Corners
		screen[y0][x0] = '+';
		screen[y0][x1] = '+';
		screen[y1][x0] = '+';
		screen[y1][x1] = '+';

		// Horizontal lines
		for (int xx=x0+1; xx<x1; xx++) screen[y0][xx] = '-';
		for (int xx=x0+1; xx<x1; xx++) screen[y1][xx] = '-';
		// Vertical lines
		for (int yy=y0+1; yy<y1; yy++) screen[yy][x0] = '|';
		for (int yy=y0+1; yy<y1; yy++) screen[yy][x1] = '|';
		*/
	
	}
	printf("    ");
	for(int i=0; i<W; i++) printf("% 3d", i);
	printf("\n   +");
	for(int i=0; i<W; i++) printf("---");
	printf("+\n");

	for(int i=0; i<L; i++) {
		printf("%2d |", i);
		for(int j=0; j<W; j++) {
			int v = screen[i][j];
			if(v == 0)
				printf("   ");
			else if(v == -1)
				printf("  *");
			else
				printf("% 3d", v);
		}
		printf("|\n");

	}
	printf("   +");
	for(int i=0; i<W; i++) printf("---");
	printf("+\n");
	




	return 0;
}
