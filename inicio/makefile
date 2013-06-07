all: temp

temp: temp.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic temp.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

nbndD12.o: temp.c smpl.h
	$(COMPILE.c) -g  temp.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o temp relat saida

