# makefile  -- <short description>
#
#  PROVIEW/R
#  Copyright (C) 1996 by Comator Process AB.
#
#  <Description>.
#
include $(pwre_dir_symbols)

ifndef variables_mk
 -include $(pwre_sroot)/tools/bld/src/$(os_name)/$(hw_name)/variables.mk
endif
ifndef variables_mk
 include $(pwre_sroot)/tools/bld/src/$(os_name)/variables.mk
endif


ifndef rules_mk
 -include $(pwre_sroot)/tools/bld/src/$(os_name)/$(hw_name)/rules.mk
endif
ifndef rules_mk
 include $(pwre_sroot)/tools/bld/src/$(os_name)/rules.mk
endif


source_dirs = $(hw_source) $(os_source) $(co_source)


.PHONY : all init copy lib exe clean realclean

all : init copy exe

init :

copy : $(inc_dir)/pwr_systemclasses.h

lib : 

exe : $(doc_dir)/pwrs_allclasses.html \
      $(exe_dir)/pwrs_xtthelp.dat

clean :

realclean : clean clean_pwrs_xtthelp clean_pwrs_html clean_systemclasses

$(inc_dir)/pwr_systemclasses.h :
	@ echo "Generating struct files for pwrs classes..."
	@ co_convert -s -d $(inc_dir) "$(pwre_sroot)/wbl/pwrs/src/pwrs_c_*.wb_load"

$(doc_dir)/pwrs_allclasses.html :
	@ echo "Generating html files for pwrs classes..."
	@ co_convert -w -d $(doc_dir) "$(pwre_sroot)/wbl/pwrs/src/pwrs_c_*.wb_load"
	@ echo "Generating html files for pwrs structs..."
	@ co_convert -c -d $(doc_dir) $(inc_dir)/pwr_systemclasses.h

$(exe_dir)/pwrs_xtthelp.dat :
	@ echo "Generating xtt help files for pwrs classes"
	@ co_convert -x -d $(exe_dir) "$(pwre_sroot)/wbl/pwrs/src/pwrs_c_*.wb_load"

#$(export_wbl) : $(cnf_dir)/%.wb_vers : %.wb_load
#       @ write sys$$output "Exporting $< ..."
#       @ $(copy) $(copyflags) $(source) $(target)
#       @ if f$$search("$(target)") .nes. "" then $(pur) $(purflags) $(target)

clean_pwrs_xtthelp :
	@ if [ -e $(exe_dir)/pwrs_xtthelp.dat ]; then \
		 $(rm) $(rmflags) $(exe_dir)/pwrs_xtthelp.dat; \
	  fi
clean_pwrs_html :
	@ if [ -e $(doc_dir)/pwrs_allclasses.html ]; then \
	        $(rm) $(rmflags) $(doc_dir)/pwrs_*.html; \
	  fi

clean_systemclasses :
	@ if [ -e $(inc_dir)/pwr_systemclasses.h ]; then \
		$(rm) $(rmflags) $(inc_dir)/pwr_systemclasses.h; \
		$(rm) $(rmflags) $(inc_dir)/pwrs_c_*.h; \
	  fi

