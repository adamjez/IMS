.PHONY: all model
all: model
model:
	g++ -std=c++11 -o model model.cc parser.cc  -lsimlib -lm 

#-g -O2