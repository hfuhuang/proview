import_top : import_files

source  	= $<
target  	= $@

import_modules = \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/pwrs.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/pwrb.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/rt.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/basecomponent.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/nmps.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/opc.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/profibus.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/otherio.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/remote.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/tlog.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/abb.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/inor.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/klocknermoeller.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/othermanufacturer.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/siemens.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/ssabox.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/telemecanique.dbs \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/rt/inc/pwr_systemclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/rt/inc/pwr_systemclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/rt/inc/pwr_baseclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/rt/inc/pwr_baseclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/bcomp/inc/pwr_basecomponentclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/bcomp/inc/pwr_basecomponentclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/nmps/inc/pwr_nmpsclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/nmps/inc/pwr_nmpsclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/opc/inc/pwr_opcclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/opc/inc/pwr_opcclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/profibus/inc/pwr_profibusclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/profibus/inc/pwr_profibusclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/otherio/inc/pwr_otherioclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/otherio/inc/pwr_otherioclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/remote/inc/pwr_remoteclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/remote/inc/pwr_remoteclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/abb/inc/pwr_abbclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/abb/inc/pwr_abbclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/inor/inc/pwr_inorclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/inor/inc/pwr_inorclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/klocknermoeller/inc/pwr_klocknermoellerclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/klocknermoeller/inc/pwr_klocknermoellerclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/othermanu/inc/pwr_othermanufacturerclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/othermanu/inc/pwr_othermanufacturerclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/siemens/inc/pwr_siemensclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/siemens/inc/pwr_siemensclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/ssabox/inc/pwr_ssaboxclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/ssabox/inc/pwr_ssaboxclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/telemecanique/inc/pwr_telemecaniqueclasses.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/telemecanique/inc/pwr_telemecaniqueclasses.hpp \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_ldh.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_wnav_selformat.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_nav.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_nav_gtk.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_pal.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_palfile.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_wccm.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_log.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_log_gtk.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_trv.h \
		$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/wb_vext.h

import_files : $(import_modules)
	@ echo ""

.SUFFIXES: 

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/load/%.dbs : $(pwre_vmsinc)/exp/load/%.dbs
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/rt/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/rt/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/bcomp/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/bcomp/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/nmps/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/nmps/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/opc/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/opc/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/profibus/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/profibus/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/otherio/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/otherio/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/remote/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/remote/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/abb/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/abb/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/inor/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/inor/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/klocknermoeller/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/klocknermoeller/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/othermanu/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/othermanu/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/siemens/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/siemens/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/ssabox/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/ssabox/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/telemecanique/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/telemecanique/inc/%.hpp : $(pwre_vmsinc)/exp/inc/%.hpp
	@ echo Import ${target}
	@ cp $(source) $(target)

$(pwre_broot)/$(pwre_os)/$(pwre_hw)/exp/inc/%.h : $(pwre_vmsinc)/exp/inc/%.h
	@ echo Import ${target}
	@ cp $(source) $(target)

