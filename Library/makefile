LFLAGS = -shared -DMoorDyn_EXPORTS -DLINUX -fPIC
MDD = MoorDyn-1.01.02/compileSO
MD = ${MDD}/MoorDyn.o ${MDD}/Line.o ${MDD}/Connection.o ${MDD}/Misc.o ${MDD}/kiss_fft.o
CFLAGS = -c -O3 -g -w -Wall -std=gnu++0x -DLINUX -fPIC


main:
	@g++ main.c ${MD} -lstdc++
	@./a.out

comp: 
	g++ -DDOUBLE_PRECISION -DLINUX -fPIC -shared -ldl Coupling/uclib.cpp ${MD} -o test.so
