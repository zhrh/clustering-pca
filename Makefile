cplus = g++	-fopenmp
cc = gcc -fopenmp
CFLAGS = -g -fPIC `pkg-config --cflags opencv` -I . -I ./libvl -I ./yael
LDFLAGS = -lblas
EXTRALIBS = `pkg-config --libs opencv` -L./yael -lyael -L./libvl -lvl 
objects+=dictionary.o utils.o pca_transform.o pca_rotation.o
main_obj+=main.o 

all:libpredata.so predata

.cpp.o:
	$(cplus) $(CFLAGS) -c $<
	
.c.o:
	$(cc) $(CFLAGS) -c $<

predata:$(libpredata.so) $(main_obj)
	$(cplus) -o $@ main.o $(EXTRALIBS) -L. -lpredata -Wl,-rpath,./

libpredata.so:$(objects)
	$(cplus) -shared -o $@ $(objects) $(LDFLAGS) $(EXTRALIBS)

clean:
	rm *.o predata libpredata.so
