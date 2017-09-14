APP_NAME := example1
INCLUDE_DIRECTORY := ../../ahocorasick
LINK_DIRECTORY := ../../ahocorasick/build
LINK_LIBRARY := ahocorasick
LINK_TARGET := $(LINK_DIRECTORY)/lib$(LINK_LIBRARY).a

ifeq ($(wildcard $(LINK_TARGET)),) 
all:;@echo 'Please go to ../../ahocorasick directory and complie it first.'
else
all: $(APP_NAME)
endif

$(APP_NAME): $(APP_NAME).o $(LINK_TARGET)
	cc -o $(APP_NAME) $(APP_NAME).o -L$(LINK_DIRECTORY) -l$(LINK_LIBRARY)

$(APP_NAME).o: $(APP_NAME).c
	cc -o $(APP_NAME).o -c $(APP_NAME).c -I$(INCLUDE_DIRECTORY) -Wall

clean:
	rm -f $(APP_NAME) $(APP_NAME).o
