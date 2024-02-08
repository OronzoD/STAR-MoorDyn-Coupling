LFLAGS = -shared -static-libgcc -static-libstdc++ -DMoorDyn_EXPORTS -DLINUX -fPIC
MDD = MoorDyn-master/compileSO
MD = ${MDD}/MoorDyn.o ${MDD}/Line.o ${MDD}/Connection.o ${MDD}/Misc.o ${MDD}/kiss_fft.o
CFLAGS = -c -O3 -g -w -Wall -static -static-libgcc -static-libstdc++ -std=gnu++0x -DLINUX -fPIC


main:
	@g++ main.c ${MD} -lstdc++
	@./a.out

comp: 
	g++ -DDOUBLE_PRECISION -fPIC -shared -ldl Coupling/uclib.cpp ${MD} -o test.so 
