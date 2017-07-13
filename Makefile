#!/bin/bash

LIBS=-lfl
CPPFLAGS=-std=c++11 -Isrc/ -Igen/
MKDIR=mkdir -p

CPPFILES=$(shell find src/ -type f | grep \\.cpp$) gen/lexer.cpp gen/parser.cpp
OBJECTS=$(patsubst gen/%.cpp,obj/%.o,$(patsubst src/%.cpp,obj/%.o,$(CPPFILES)))
GENERATED=gen/lexer.cpp gen/parser.cpp gen/parser.hpp
DEPENDENCY=$(patsubst obj/%.o,dep/%.d,$(OBJECTS))

all: dep obj gen glslc

clean:
	$(RM) -r gen obj dep glslc

-include $(DEPENDENCY)

glslc: $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

gen/parser.hpp gen/parser.cpp: src/parser.y
	bison --defines=gen/parser.hpp -o gen/parser.cpp $<

gen/lexer.cpp: src/lexer.l
	flex -o $@ $<

dep obj gen:
	$(MKDIR) $@

obj/%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) -g -c -o $@ $< -MMD -MF $(patsubst obj/%.o,dep/%.d,$@)

obj/%.o: gen/%.cpp
	$(CXX) $(CPPFLAGS) -g -c -o $@ $< -MMD -MF $(patsubst obj/%.o,dep/%.d,$@)


obj/driver.o: gen/parser.hpp



