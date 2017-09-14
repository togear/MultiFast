APP_NAME := example3
INCLUDE_DIRECTORY := ../../ahocorasick
LINK_DIRECTORY := ../../ahocorasick/build
LINK_LIBRARY := ahocorasick
LINK_TARGET := $(LINK_DIRECTORY)/lib$(LINK_LIBRARY).a
HEADER_FILES := AhoCorasickPlus.h

ifeq ($(wildcard $(LINK_TARGET)),) 
all:;@echo 'Please go to ../../ahocorasick directory and complie it first.'
else
all: $(APP_NAME)
endif

$(APP_NAME): $(APP_NAME).o AhoCorasickPlus.o $(LINK_TARGET)
	g++ -o $(APP_NAME) $(APP_NAME).o AhoCorasickPlus.o -l$(LINK_LIBRARY) -L$(LINK_DIRECTORY)

$(APP_NAME).o: $(APP_NAME).cpp $(HEADER_FILES)
	g++ -o $(APP_NAME).o -c $(APP_NAME).cpp -I$(INCLUDE_DIRECTORY) -Wall 

AhoCorasickPlus.o: AhoCorasickPlus.cpp $(HEADER_FILES)
	g++ -o AhoCorasickPlus.o -c AhoCorasickPlus.cpp -I$(INCLUDE_DIRECTORY) -Wall

clean:
	rm -rf $(APP_NAME) $(APP_NAME).o AhoCorasickPlus.o
