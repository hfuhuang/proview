
ifndef variables_mk
 -include $(pwre_sroot)/tools/bld/src/$(os_name)/$(hw_name)/variables.mk
endif
ifndef variables_mk
 include $(pwre_sroot)/tools/bld/src/$(os_name)/variables.mk
endif


vpath %.uil $(hw_source):$(os_source):$(co_source)

source_dirs := $(hw_source) $(os_source) $(co_source)

uil_sources := $(sort \
             $(foreach file, \
               $(foreach dir, \
                 $(source_dirs), \
                 $(wildcard $(dir)/*.uil) \
               ), $(notdir $(file)) \
             ) \
           )

export_uid_sv_se := $(addprefix $(exe_dir)/sv_se/, $(patsubst %.uil, %.uid, $(uil_sources)))
export_uid_en_us := $(addprefix $(exe_dir)/en_us/, $(patsubst %.uil, %.uid, $(uil_sources)))

export_uid := $(export_uid_sv_se) $(export_uid_en_us)
clean_uid := $(patsubst %.uil,clean_%.uid,$(uil_sources))

.SUFFIXES:

$(exe_dir)/sv_se/%.uid : %.uil
	@ $(log_uil_uid) sv_se
	@ uil -o $(target) -I$(pwre_sroot)/$(type_name)/$(comp_name)/sv_se $(source)

$(exe_dir)/en_us/%.uid : %.uil
	@ $(log_uil_uid) en_us
	@ uil -o $(target) -I$(pwre_sroot)/$(type_name)/$(comp_name)/en_us $(source)

.PHONY : all init copy lib exe clean realclean

all : init copy lib exe

init :

copy : $(export_uid)

lib :

exe :


clean : clean_uil

realclean : clean

$(clean_uid) : clean_%.uid : %.uil
	@ echo "Removing uid"
	@ $(rm) $(rmflags) $(exe_dir)/sv_se/$*.uid 
	@ $(rm) $(rmflags) $(exe_dir)/en_us/$*.uid 




