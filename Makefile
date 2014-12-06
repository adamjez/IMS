.PHONY: all model
all: model
model:
	g++ -std=c++11 -o model parser.cc model.cc  -lsimlib -lm 

#-g -O2