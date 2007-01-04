ifndef link_rule_mk
link_rule_mk := 1

link = $(ldxx) $(elinkflags) $(domap) -o $(pwr_exe)/wb_ge_motif \
	$(bld_dir)/wb_ge_motif.o $(wb_msg_eobjs) $(rt_msg_eobjs) \
	$(pwr_eobj)/rt_io_user.o $(pwr_eobj)/wb_provider.o -L/usr/X11R6/lib \
	-L/usr/local/BerkeleyDB.4.0/lib \
	-L/opt/gnome/lib \
	-lpwr_ge_motif -lpwr_ge -lpwr_wb_motif -lpwr_wb -lpwr_wb_motif -lpwr_wb -lpwr_flow_motif -lpwr_flow \
	-lpwr_glow_motif -lpwr_glow -lpwr_rt_motif -lpwr_rt -lpwr_co_motif -lpwr_co \
	-lpwr_msg_dummy -lantlr -lImlib -lMrm -lXm -lXpm -lXt -lX11 -lXext -lXp\
        -lXmu -lSM -lICE\
	-lrpcsvc -lpthread -lm -ldb_cxx -lz

endif
