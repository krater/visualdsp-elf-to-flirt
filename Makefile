CC := g++
BINNAME := doj2map
INPUTFILES := doj2map.cpp functionblocks.cpp dlbreader.cpp dojreader.cpp

# Stuff we need
INCLUDECFLAGS := #`pkg-config --cflags sdl`
INCLUDELIBFLAGS := -I"." -lelf 
INCLUDEFLAGS := $(INCLUDECFLAGS) $(INCLUDELIBFLAGS)

# Flags in common by all
CFLAGS := -Wall -W -Wextra -pedantic -pedantic-errors -Wfloat-equal -Wundef -Wshadow -Winit-self \
-Wpointer-arith -Wcast-align -Wwrite-strings -Wcast-qual -Wvla\
-Wswitch-enum -Wconversion -Wformat=2 -Wswitch-default -Wstrict-overflow -std=c++0x

# Flags for debugging builds
CDFLAGS := $(CFLAGS) -g -O2 -fstack-protector-all -Wstack-protector -Wstrict-overflow=4
# Flags for normal builds
CNFLAGS := $(CFLAGS) -O3 -fno-stack-protector
# Flags for very aggressive builds
COFLAGS := $(CFLAGS) -O3 -ffast-math -funsafe-loop-optimizations -Wunsafe-loop-optimizations \
-fstrict-aliasing -Wstrict-aliasing -fstrict-overflow

SSEFLAGS := -msse2 -mfpmath=sse

# This stuff is kinda extreme...
EXTREMEFLAGS := -funsafe-math-optimizations -ffinite-math-only -funroll-all-loops

# Profiling stuff:
# -fbranch-probabilities (look more into that kind of stuff) -fpeel-loops -fprofile-use

# Other interesting stuff to look into:
# -floop-block -floop-interchange -floop-strip-mine -fprefetch-loop-arrays


all: default 
default: clean
	@$(CC) $(CDFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -o $(BINNAME)
default-sse: clean
	@$(CC) $(SSEFLAGS) $(CNFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -o $(BINNAME)
debug: clean
	@$(CC) $(CDFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -o $(BINNAME)
optimized: clean
	@$(CC) $(COFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -o $(BINNAME)
optimized-sse: clean
	@$(CC) $(SSEFLAGS) $(COFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -o $(BINNAME)
extremely-optimized: clean
	@$(CC) $(COFLAGS) $(EXTREMEFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -o $(BINNAME)
run:
	@time ./$(BINNAME)
clean:
	@$(RM) $(BINNAME)
	@$(RM) *.plist


# check-syntax target for flymake.
check-syntax:
	$(CC) $(COFLAGS) $(INCLUDEFLAGS) $(INPUTFILES) -S -o /dev/null

