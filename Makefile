# @author :tzihad

SHELL       = /bin/bash

# Customize here
compiler	= g++
version		= -std=gnu++2b
warnings	= -Wall -Wextra -pedantic -Wno-long-long 
extra_args	= 
debug_args	= -g3 -DDEBUG -g
release_args=

linker		= g++
linker_args	= 

project_name= RPG
build_dir	= build
src_dirs = Graphics Engine Database
src			= $(wildcard *.cxx) $(foreach dir,$(src_dirs),$(wildcard $(dir)/*.cxx))
#this headers aren't required properly
libraries	= ncurses


#Do not edit


#### Functions
setvar	   = $(foreach A,$2,$(eval $1: $A))
addprefix  = $(foreach i,$2,$1$i)




#### Definition list
bin_dir	   =$(build_dir)/bin
obj_dir	   =$(build_dir)/obj
obj_out_dir=$(obj_dir)/debug

executable=$(bin_dir)/debug/$(project_name)
objs 	   = $(addprefix $(obj_out_dir)/,$(src:.cxx=.o))
obj 	   = $(src:.cxx=.o)
headers  = $(addsuffix /header.hxx, $(src_dirs))



alllibs =$(call addprefix,-l,$(libraries))


#### Working Variables
obj_base_args   = -c  $(warnings) $(extra_args)
obj_args=$(obj_base_args) $(debug_args) $(version)

linking_args = $(linker_args) #missing?



ifeq ($(filter release,$(MAKECMDGOALS)),release)
    executable=$(bin_dir)/release/$(project_name)
    obj_out_dir=$(obj_dir)/release
	obj_args=$(obj_base_args) $(release_args)
endif


####


.PHONY: debug build release launch clean prepare clean_raw rpg dbtest



$(obj_out_dir)/%.o : %.cxx
	$(compiler) $(obj_args) $< -o$@

$(executable) : $(objs)
	$(compiler) $(alllibs) $(warnings) $(linking_args)  -o $@ $(objs)

# Check header for each dir
define check_headers
$(1)/%.cxx: $(1)/header.hxx
	@ touch $$@
endef
$(foreach dir,$(src_dirs),$(eval $(call check_headers,$(dir))))

build: $(executable)
release: $(executable)
launch: build
	@ $(executable)
run: build
	@ $(executable)
debug: build
	gdb $(executable) --tui

# Build and run the open-world RPG (Perlin + cellular automata chunks).
RPG_DIR = RPG
rpg:
	$(MAKE) -C $(RPG_DIR) run

# Run the database test harness.
dbtest:
	$(MAKE) -C Database/test -f Makefile-test
	./Database/test/build/dbtest
prepare:
	mkdir -p $(build_dir)
	mkdir -p $(bin_dir)
	mkdir -p $(bin_dir)/release
	mkdir -p $(bin_dir)/debug
	mkdir -p $(obj_dir)
	mkdir -p $(obj_dir)/release
	mkdir -p $(obj_dir)/debug
	mkdir -p $(foreach dir,$(src_dirs),$(obj_dir)/release/$(dir))
	mkdir -p $(foreach dir,$(src_dirs),$(obj_dir)/debug/$(dir))

clean_raw:
	@ rm -rf build

clean: clean_raw prepare