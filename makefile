OSG_LIBS = -losgViewer -losg -losgGA -losgDB -losgText -losgSim -lOpenThreads  -lboost_regex -losgWidget 
CFLAGS= -Wno-deprecated -Wall
OBJS= KeyboardEventHandler.o Package.o 

Debviewer3d: Debviewer3d.cpp KeyboardEventHandler.o Package.o 
	g++  -g  -o debviewer3d Debviewer3d.cpp $(OBJS) $(OSG_LIBS) $(CFLAGS)

KeyboardEventHandler.o: KeyboardEventHandler.cpp
	g++ -c KeyboardEventHandler.cpp

Package.o: Package.cpp Package.h
	g++ -c Package.cpp  

clean:
	rm -f debviewer3d *.o

