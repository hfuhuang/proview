#  variables.mk -- variables common to all makefiles
#
#  PROVIEW/R
#  Copyright (C) 1996 by Comator Process AB.
#
#  <Description>
#
ifndef variables_mk
variables_mk := 1

release_root	:= $(pwre_broot)/$(os_name)/$(hw_name)
ifneq ($(type_name),tst)
  exp_root	:= $(release_root)/exp
else
  exp_root	:= $(release_root)/tst
  imp_dir	:= $(release_root)/exp/inc
endif

exe_dir	:= $(exp_root)/exe
lib_dir	:= $(exp_root)/lib
obj_dir	:= $(exp_root)/obj

cnf_dir	:= $(exp_root)/cnf
db_dir	:= $(exp_root)/db
inc_dir	:= $(exp_root)/inc
lis_dir	:= $(exp_root)/lis
kit_dir	:= $(exp_root)/kit
src_dir	:= $(exp_root)/src

bld_dir	:= $(release_root)/bld/$(type_name)/$(comp_name)
tmp_dir	:= $(release_root)/tmp

c_ext	:= .c
d_ext	:= .d
d_ld_ext:= .d_ld
dir_ext	:= .dir
exe_ext	:= .exe
h_ext	:= .c
lib_ext	:= .olb
lis_ext	:= .lis
map_ext	:= .map
obj_ext	:= .obj
opt_ext	:= .opt

#  Full names of source directories

fhw_source := $(pwre_sroot)/$(type_name)/$(comp_name)/src/$(os_name)/$(hw_name)
fos_source := $(pwre_sroot)/$(type_name)/$(comp_name)/src/$(os_name)
fco_source := $(pwre_sroot)/$(type_name)/$(comp_name)/src

#  Realtive names of source directories

hw_source := .
os_source := ../
co_source := ../../


ifndef pwre_lang
  lang_dir := $(exe_dir)
  lang_bld_dir := $(bld_dir)
else
  ifeq ($(pwre_lang),en_us)
    lang := en
  endif

  ifeq ($(pwre_lang),sv_se)
    lang     := sw
    pwr_lang := 1
  endif

  lang_hw_source := $(pwre_sroot)/$(type_name)/$(comp_name)/$(pwre_lang)/$(os_name)/$(hw_name)
  lang_os_source := $(pwre_sroot)/$(type_name)/$(comp_name)/$(pwre_lang)/$(os_name)
  lang_co_source := $(pwre_sroot)/$(type_name)/$(comp_name)/$(pwre_lang)
  lang_dir := $(exe_dir)/$(pwre_lang)
  lang_bld_dir := $(bld_dir)/$(pwre_lang)
endif

source	= $(to-vms $<)
sname	= $(notdir $(basename $<))
ssuff	= $(suffix $<)
sdir	= $(to-vms $(dir $<)/)
target	= $(to-vms $@)
tname	= $(notdir $(basename $@))
tsuff	= $(suffix $@)
tdir	= $(to-vms $(dir $@)/)

libobj	= $(to-vms $(bld_dir)/$*$(obj_ext))
cobj	= /obj=$(target)
clibobj	= /obj=$(libobj)

tdepmms = $(to-vms $(tmp_dir)/$(sname).mms)
depmms  = /mms=(nosys,file=$(tdepmms))
depaddpath := 1
depaddlib := 2

log_done	= \@pwr_tools:pwr_log_done
#clib		:= +sys$library:vaxcdef/lib+pams$$lib:pams/lib
#clib		:= +dmq$$user:dmq/lib
clib		:= 
csetos		:= /define=(os_vms=1,os="vms",hw_vax=1,hw="vax")
ifneq ($(type_name),tst)
  cinc		:= /inc=($(to-vms $(inc_dir)/),$(to-vms $(hw_source)/),\
		$(to-vms $(os_source)/),$(to-vms $(co_source)/))
  cbldinc       := /inc=($(to-vms $(inc_dir)/),$(to-vms $(hw_source)/),\
                $(to-vms $(os_source)/),$(to-vms $(co_source)/),\
                $(to-vms $(bld_dir)/))
  scinc		:= $(to-vms $(inc_dir)/),$(to-vms $(fhw_source)/),$(to-vms $(fos_source)/),$(to-vms $(fco_source)/)
else
  cinc		:= /inc=($(to-vms $(inc_dir)/),$(to-vms $(imp_dir)/),$(to-vms $(hw_source)/),\
		$(to-vms $(os_source)/),$(to-vms $(co_source)/))
  cbldinc       := /inc=($(to-vms $(inc_dir)/),,$(to-vms $(imp_dir)/)$(to-vms $(hw_source)/),\
                $(to-vms $(os_source)/),$(to-vms $(co_source)/),\
                $(to-vms $(bld_dir)/))
  scinc		:= $(to-vms $(inc_dir)/),$(to-vms $(imp_dir)/),$(to-vms $(fhw_source)/),$(to-vms $(fos_source)/),$(to-vms $(fco_source)/)
endif
rm		:= delete
rmflags		:= /noconfirm/nolog
expand_opt	:= \@pwr_exe:pwr_expand_opt
gm1		:= \@pwr_exe:pwr_gm1
cp		:= copy
cpflags		:= /noconfirm/nolog
link		:= link
pur             := purge
purflags        := /noconfirm/nolog
ar		:= library
arflags		:= /replace
arflags_cre	:= /create=(modu:200)/log
obj             = /obj=$(target)
uilflags        := /motif
sqlmod		:= mc sql$$mod

cc              := cc/decc
cxx             := cxx
scc             := $$sql$$pre/cc=decc

list            = $(to-vms $(lis_dir)/$(sname)$(lis_ext))
# The command is too long for dep_map2make, use pwr_lis instead.
#map             = $(to-vms $(lis_dir)/$(sname)$(map_ext))
map             = pwr_lis:$(sname)$(map_ext)
mkdir		:= create/directory
mkdirflags	:= /log

ifeq ($(pwre_btype),rls)
  cflags	:= /warn=dis=TRAILCOMMA/optimize/debug=trace/float=d_float/extern_m=strict/stan=rel/prefix=all_entries/define=(PWR_NDEBUG=1)
  cxxflags	:= /warn=dis=TRAILCOMMA/opt/deb=tra/flo=d_flo/exte=str/nomem/pref=all
  scflags	:= /opt/deb=tra/nog_flo/extern_m=stri/stan=rel/pref=all/warn=dis=TRAILCOMMA
  pflags	:= /opt/debug=traceback
  linkflags	:= /nodebug/trace
  clis		= /lis=$(list)
  dolist	= /lis=$(list)
  domap		= /map=$(map)/full
else
  cflags	:= /warn=dis=TRAILCOMMA/nooptimize/debug/float=d_float/extern_m=strict/stan=rel/prefix=all_entries
  cxxflags	:= /warn=dis=TRAILCOMMA/rep=pwrb_root:[cxx_demang]/noopt/deb/flo=d_flo/exte=str/nomem/pref=all/def=(THREADISHOME=1)
  scflags	:= /noopt/debu/nog_flo/extern_m=stri/stan=rel/pref=all/warn=dis=TRAILCOMMA
  pflags	:= /noopt/debug
  linkflags	:= /debug
  dolist	= /lis=$(list)
  clis		:=
ifeq ($(nodep),)
  domap		= /map=$(map)/brief
else
  domap		:=
endif

endif

log_h_h		= write sys$$output "Exporting $<"
log_x_h		= write sys$$output "Exporting $< to $(notdir $@)"
log_c_obj	= write sys$$output "Compiling $<"
log_c_d		= write sys$$output "Updating dependencies $<"
log_c_lib	= write sys$$output "Compiling $(notdir $(basename $@))($<)"
log_x_lib	= write sys$$output "Compiling $(notdir $(basename $@))($<)"
log_obj_lib	= write sys$$output "Archiving $(notdir $(basename $@))($(notdir $<))"
log_hlp_hlb	= write sys$$output "Exporting $(notdir $(basename $@))($<)"
log_sqlmod_lib	= write sys$$output "Compiling $(notdir $(basename $@))($<)"
log_sqlmod_obj	= write sys$$output "Compiling $<"
log_sc_lib	= write sys$$output "Compiling $(notdir $(basename $@))($<)"
log_sc_obj	= write sys$$output "Compiling $<"
log_uil_uid	= write sys$$output "Compiling $<"
log_g_m1	= write sys$$output "Exporting $<"
log_cld_lib	= write sys$$output "Compiling $(notdir $(basename $@))($<)"
log_link_exe	= write sys$$output "Kitlinking $@"
log_opt_exe	= write sys$$output "Linking $@"
log_wb_load_wb_vers	= write sys$$output "Exporting $<"
log_xbm_xbm	= write sys$$output "Exporting $<"
log_frm_frm	= write sys$$output "Exporting $<"
log_graph_graph	= write sys$$output "Exporting $<"
log_msg_h	= write sys$$output "Compiling $<"

endif
