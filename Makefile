INCLUDE = ../v8
LIBDIR = ../v8/out/native
CC = g++
LDFLAGS = -lpthread -mmacosx-version-min=10.8 -std=c++0x
OUTDIR = build

V8_LIBS = base libplatform snapshot libbase
ICU_LIBS = data i18n uc
LIBPATHS = \
	$(patsubst %,$(LIBDIR)/libv8_%.a, $(V8_LIBS)) \
	$(patsubst %,$(LIBDIR)/libicu%.a, $(ICU_LIBS)) \

build: hello_world.cc
	mkdir -p $(OUTDIR); \
	$(CC) -I$(INCLUDE) hello_world.cc -o $(OUTDIR)/hello_world \
	$(LIBPATHS) $(LDFLAGS)
