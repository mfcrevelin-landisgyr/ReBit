N = 21

dfa = [[ 0] *256]*N

for c in range(256):
	dfa[ 0][c] =  0
	dfa[ 1][c] =  0
	dfa[ 2][c] =  0
	dfa[ 5][c] =  0
	
	dfa[ 3][c] =  7
	dfa[ 6][c] =  7
	dfa[ 7][c] =  7
	dfa[ 8][c] =  7
	dfa[ 9][c] =  7
	dfa[10][c] =  7
	dfa[11][c] =  7
	dfa[13][c] =  7

	dfa[ 4][c] = N - 1
	dfa[12][c] = N - 1
	dfa[14][c] = N - 1
	dfa[15][c] = N - 1
	dfa[16][c] = N - 1
	dfa[17][c] = N - 1
	dfa[18][c] = N - 1
	dfa[19][c] = N - 1
	dfa[-1][c] = N - 1

for c in [ord(i) for i in ['\t',' ']]:
	dfa[ 3][c] =  6
	dfa[ 6][c] =  6

for c in [ord('_')]+[ord('a')+i for i in range(26)]+[ord('A')+i for i in range(26)]+[ord('0')+i for i in range(10)]:
	dfa[12][c] = 13
	dfa[13][c] = 13

for c in [i for i in range(33,127)]:

	dfa[14][c] = 15
	dfa[15][c] = 15
	dfa[16][c] = 15
	dfa[17][c] = 15
	dfa[18][c] = 15
	dfa[19][c] = 15

dfa[ 0][ord('\n')] =  1
dfa[ 1][ord('\n')] =  1
dfa[ 2][ord('\n')] =  1
dfa[ 5][ord('\n')] =  1

dfa[ 4][ord('\n')] = 14

dfa[ 3][ord('\n')] = N - 1
dfa[ 6][ord('\n')] = N - 1
dfa[ 7][ord('\n')] = N - 1
dfa[ 8][ord('\n')] = N - 1
dfa[ 9][ord('\n')] = N - 1
dfa[10][ord('\n')] = N - 1
dfa[11][ord('\n')] = N - 1
dfa[12][ord('\n')] = N - 1
dfa[13][ord('\n')] = N - 1

dfa[14][ord('\n')] = 14
dfa[15][ord('\n')] = 14
dfa[16][ord('\n')] = 14
dfa[17][ord('\n')] = 14
dfa[18][ord('\n')] =  1
dfa[19][ord('\n')] = 14

dfa[ 3][ord('v')] =  8
dfa[ 6][ord('v')] =  8
dfa[ 7][ord('v')] =  8
dfa[ 8][ord('a')] =  9
dfa[ 9][ord('r')] = 10
dfa[ 9][ord('l')] = 11
dfa[10][ord('_')] = 12
dfa[11][ord('_')] = 12

dfa[ 0][ord('$')] =  2
dfa[ 1][ord('$')] =  2
dfa[ 2][ord('$')] =  3
dfa[ 3][ord('$')] =  4
dfa[ 4][ord('$')] =  5
dfa[ 5][ord('$')] =  5

dfa[14][ord('$')] = 16
dfa[15][ord('$')] = 16
dfa[16][ord('$')] = 17
dfa[17][ord('$')] = 18
dfa[18][ord('$')] = 19
dfa[19][ord('$')] = 19

