CFG ?= Debug

CC := g++
CFLAGS := -c -std=c++14 -Wall -DGTE_USE_LINUX -DGTE_USE_ROW_MAJOR -DGTE_USE_MAT_VEC -DGTE_USE_OPENGL -DGTE_DISABLE_PCH #-Werror

ifeq (Debug,$(findstring Debug,$(CFG)))
CFLAGS += -g -D_DEBUG
else
CFLAGS += -O2 -DNDEBUG -fopenmp
endif

ifdef INSPECT
CFLAGS += -DINSPECT
endif

INCPATH := -I $(GTE4_PATH)
SRC := $(notdir $(wildcard *.cxx))
OBJ := $(SRC:%.cxx=$(CFG)/%.o)
LIBPATH := -L $(GTE4_PATH)/lib/$(CFG)
LDFLAGS := -Wl,--no-as-needed
LIBS := -lgtapplications -lgtgraphics -lgtmathematicsgpu -lX11 -lXext -lGL -lEGL -lpng -lpthread -lm -lgomp
LIBS += -lnlopt

build : createdir compile

createdir :
	@mkdir -p $(CFG)

compile : $(OBJ)
	$(CC) $(LDFLAGS) $(LIBPATH) $(OBJ) -o $(APP).$(CFG) $(LIBS)

clean :
	rm -rf $(CFG)
	rm -f $(APP).$(CFG)

$(CFG)/%.o : %.cxx
	$(CC) $(CFLAGS) $(INCPATH) $< -o $@
