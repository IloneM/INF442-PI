CXX = mpic++
#CXX = g++
RM = rm -f

# object files it depends on
OBJS = tools.o feature.o integralimg.o trainer.o
BIN = train

LATEXTARGET = rapport
LATEXFILE = $(LATEXTARGET).tex
LATEXOBJS = $(patsubst %.tex,%.aux,$(LATEXFILE)) $(patsubst %.tex,%.log,$(LATEXFILE))
LATEXOUT = $(patsubst %.tex,%.pdf,$(LATEXFILE)) $(patsubst %.tex,%.ps,$(LATEXFILE))
LATEXBIN = pdflatex

# remove command
DEBUG = 1        # 1=YES, [empty] = no
SETUP = ubuntu # possibilities: macosx, ubuntu, sallesinfo

DEFINES =

# flags
FLAGS = $(DEFINES)
ifdef DEBUG
  FLAGS += -g
else
  FLAGS += -O3
endif

ifeq "$(SETUP)" "sallesinfo "
  ## salles informatiques @X
  $(info building for $(SETUP))
  INCLUDE = -I/usr/local/CImg-1.6.2/
  LIBS = -lpthread -lX11
  LDPATH = LD_LIBRARY_PATH=/usr/local/boost-1.58.0/lib:/usr/lib/alliance/lib
endif

ifeq "$(SETUP)" "ubuntu "
  # ubuntu linux
  $(info building for $(SETUP))
  INCLUDE = -L/usr/X11R6/lib
  LIBS = -lpthread -lX11 -lm
  LDPATH =
endif

ifeq "$(SETUP)" "macosx "
  # macosx
  $(info building for $(SETUP))
  INCLUDE = -I/usr/local/include -I/opt/X11/include
  LIBS = -lpthread -L/usr/local/lib -L/opt/X11/lib -lX11
  DEFINES += -std=c++0x
  LDPATH = DYLD_LIBRARY_PATH=/usr/local/lib
endif

all: $(BIN) Makefile

# rule to build the executable
$(BIN): $(OBJS) $(BIN).cpp Makefile
	$(CXX) $(FLAGS) $(INCLUDE) -o $(BIN) $(BIN).cpp $(OBJS) $(LIBS)

cleanbin:
	$(RM) $(OBJS)

cleanlatex:
	$(RM) $(LATEXOBJS)

clean: cleanlatex cleanbin

totclean: clean
	$(RM) $(BIN) $(LATEXOUT)

trainer.o: trainer.cpp trainer.h perceptron.h Makefile
	$(CXX) $(FLAGS) $(INCLUDE) -c -o $@ $<

%.o:	%.cpp %.h Makefile
	$(CXX) $(FLAGS) $(INCLUDE) -c -o $@ $<

$(LATEXTARGET): $(LATEXFILE) Makefile
	$(LATEXBIN) $<

