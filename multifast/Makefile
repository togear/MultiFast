APP_NAME := multifast
BUILD_DIRECTORY := build/
APP_TARGET := $(BUILD_DIRECTORY)$(APP_NAME)
CFLAGS := -Wall -DMF_VERSION_NUMBER=2.0.0
INCLUDE_DIRECTORY := -I../ahocorasick
LINK_DIRECTORY := ../ahocorasick/build
LINK_LIBRARY := ahocorasick
LINK_TARGET := $(LINK_DIRECTORY)/lib$(LINK_LIBRARY).a
HERADER_FILES := $(wildcard *.h)
OBJECT_FILES := $(addprefix $(BUILD_DIRECTORY),$(patsubst %.c,%.o,$(wildcard *.c)))
COMPILER := cc

.PHONY : clean

ifeq ($(wildcard $(LINK_TARGET)),) 
all:;@echo 'Please go to ../ahocorasick directory and complie it first.'
else
all: $(APP_TARGET)
endif

$(APP_TARGET): $(BUILD_DIRECTORY) $(OBJECT_FILES) $(LINK_TARGET)
	$(COMPILER) -o $@ $(BUILD_DIRECTORY)*.o -L$(LINK_DIRECTORY) -l$(LINK_LIBRARY)

$(BUILD_DIRECTORY)%.o: %.c $(HEADER_FILES)
	$(COMPILER) -o $@ -c $< $(CFLAGS) $(INCLUDE_DIRECTORY)

$(BUILD_DIRECTORY):
	@mkdir -p $(BUILD_DIRECTORY)

clean:
	rm -f $(OBJECT_FILES) $(APP_TARGET)
