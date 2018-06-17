import numpy as np
import itertools
import subprocess
import time, sys

class Problem:

	def __init__(self):
		self.varindex = 1
		self.clauses = []

	def __iadd__(self, clause):
		self.clauses.append(clause)
		return self

	def new_var(self):
		var = self.varindex
		self.varindex += 1

		return var

	def __str__(self):
		return str(self.clauses)

class Solution:

	def __init__(self, problem):
		self.problem = problem
		self.vars = {}
		self.timelimit = 120
		self.status = None

	def write_problem(self, f):
		problem = self.problem
		clauses = problem.clauses
		nclauses = len(clauses)
		nvars = problem.varindex - 1
		header = "p cnf {} {}\n".format(nvars, nclauses)
		f.write(header.encode())

		for clause in clauses:
			line = " "
			for lit in clause:
				line += " {}".format(lit)
			line += " 0\n"
			f.write(line.encode())


	def solve(self):
		cmd = ["lingeling"]
		cmd += ["-t", str(self.timelimit)]
		proc = subprocess.Popen(cmd, stdin=subprocess.PIPE,
				stdout=subprocess.PIPE)

		stdin = proc.stdin
		self.write_problem(stdin)
		stdin.close()
		proc.wait()

		for line in proc.stdout:
			line = line.decode().strip()
			#print(line)
			parts = line.split()
			if parts[0] == 's':
				self.status = parts[1]

			elif parts[0] == 'v':
				var_values = parts[1:]
				for v in var_values:
					num = int(v)
					if num == 0: continue
					self.vars[num] = True
					self.vars[-num] = False

		ret = proc.returncode

		#print(self.status)
		#print(self.vars)
	
	def __getitem__(self, key):
		#print("Var {} is {}".format(key, self.vars[key]))
		return self.vars[key]

		



class Boxes:
	def __init__(self):
		self.p = Problem()
		self.read_instance()
		self.build()
		self.solve()

	def read_instance(self):
		W = int(input())
		num_boxes = 0
		w = []
		h = []

		max_L = 0

		try:
			while True:
				line = input()
				parts = line.split()
				num = int(parts[0])
				wi = int(parts[1])
				hi = int(parts[2])
				w += [wi] * num
				h += [hi] * num
				max_L += num * max(wi, hi)
				num_boxes += num

		except EOFError:
			pass

		self.w = w
		self.h = h
		self.N = num_boxes
		self.L = max_L
		self.W = W

	def matrix3d(self, name, bounds):
		var = np.empty(bounds, dtype='int')
		for i in range(bounds[0]):
			for j in range(bounds[1]):
				for k in range(bounds[2]):
					var[i,j,k] = self.p.new_var()
					
		return var

	def matrix1d(self, name, bounds):
		var = np.empty(bounds, dtype='int')
		for i in range(bounds[0]):
			var[i] = self.p.new_var()
		return var


	def box_size(self, i, rot):
		"Real size of the box i after rotation"
		h = self.h
		w = self.w

		if rot:
			return h[i], w[i]
		
		return w[i], h[i]

	def domain_all(self):
		"All cells that can be used"
		range_x = range(0, self.W)
		range_y = range(0, self.L)

		return list(itertools.product(range_x, range_y))

	def domain_start(self, i, rot):
		"All cells that can be used as start for the box i"
		w, h = self.box_size(i, rot)
		range_x = range(0, self.W - w + 1)
		range_y = range(0, self.L - h + 1)

		return list(itertools.product(range_x, range_y))

	def domain_box(self, i, rot, s):
		"Return the set of cells occupied by the box i placed at s"
		x, y = s
		w, h = self.box_size(i, rot)
		range_x = range(x, x + w)
		range_y = range(y, y + h)

		return list(itertools.product(range_x, range_y))

	def box_constraint_rotation(self, i, rot):
		p = self.p
		S = self.S
		P = self.P

		if rot == False:
			#TODO Revise rot an Ri
			Ri = -self.R[i]
		else:
			Ri = self.R[i]

		# Ri is true if R[i] == rot

		force_start = [-Ri]
		starts = self.domain_start(i, rot)
		for start in starts:
			sx,sy = start
			cells = self.domain_box(i, rot, start)
			#print('Cells for box={} (w={}, h={}) with rot={} at s={}: {}'.format(
			#	i, self.w[i], self.h[i], rot, start, cells))
			#time.sleep(10)
			for cx, cy in cells:
				# Assume no rotation

				#self.exp &= Ri >> (self.S[sx,sy,i] >> self.P[cx, cy, i])
				p += [-Ri, -S[sx,sy,i], P[cx, cy, i]]

			# Force at least one start per box
			force_start.append(self.S[sx,sy,i])

		p += force_start
		
		#self.exp &= [Ri] + >> (force_start)

		all_cells = set(self.domain_all())
		starts_set = set(starts)
		non_starts = all_cells - starts_set

		#print("all_cells", all_cells)
		#print("starts", starts_set)
		#print("non starts", non_starts)


		# Deny all other posible cells as start point
		#for x,y in non_starts:
		#	self.exp &= Ri >> (-self.S[x,y,i])

		# But at most one
		nstarts = len(starts)
		for j in range(nstarts):
			for k in range(nstarts):
				if j >= k: continue
				xj, yj = starts[j]
				xk, yk = starts[k]

				#self.exp &= Ri >> (-self.S[xj, yj, i] | -self.S[xk, yk, i])
				#self.exp &= -Ri | -self.S[xj, yj, i] | -self.S[xk, yk, i]
				p += [-Ri, -S[xj, yj, i], -S[xk, yk, i]]

	def box_constraint(self, i):
		self.box_constraint_rotation(i, False)
		self.box_constraint_rotation(i, True)

	def cell_constraint(self, cell):
		x, y = cell
		N = self.N
		P = self.P
		p = self.p

		for i in range(N):
			for j in range(N):
				if i >= j: continue

				# At most one cell occupied (quadratic encoding)
				# No overlapping
				#self.exp &= (-P[x,y,i] | -P[x,y,j])

				p += [-P[x,y,i], -P[x,y,j]]
				

	def build(self):
		N = self.N
		W = self.W
		L = self.L

		self.R = self.matrix1d('R', [N])
		self.P = self.matrix3d('P', [W, L, N])
		self.S = self.matrix3d('S', [W, L, N])

		R = self.R
		P = self.P
		S = self.S


		for i in range(N):
			#print("Building constraints for box {}".format(i),
			#	file=sys.stderr)
			self.box_constraint(i)

		for x in range(W):
			for y in range(L):
				cell = (x, y)
				#print("Building constraint for cell {}".format(cell),
				#	file=sys.stderr)
				self.cell_constraint(cell)

	def reduce_length(self):
		L = self.L
		P = self.P
		p = self.p

		y = L-1
		for x in range(self.W):
			for i in range(self.N):
				p += [-P[x,y,i]]

		self.L -= 1

	def solve(self):

		best_sol = None

		while True:

			sol = Solution(self.p)
			sol.solve()

			if sol.status != 'SATISFIABLE': break

			print('Found solution with L = {}'.format(self.L), file=sys.stderr)

			best_sol = sol

			self.reduce_length()

		# Last reduction was not satisfiable, we need to increase L to the last
		# value
		self.L += 1

		print("{} {}".format(self.W, self.L))

		for i in range(self.N):
			for x in range(self.W):
				for y in range(self.L):
					placed = best_sol[self.S[x,y,i]]
					rot = best_sol[self.R[i]]

					if placed:
						w, h = self.box_size(i, rot)
						print("{} {}    {} {}".format(x, y, x+w-1, y+h-1))



b = Boxes()
