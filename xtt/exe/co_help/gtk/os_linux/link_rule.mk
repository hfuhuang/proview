ifndef link_rule_mk
link_rule_mk := 1

  link = $(ldxx) $(elinkflags) $(domap) -o $(pwr_exe)/co_help_gtk \
	$(bld_dir)/co_help_gtk.o $(bld_dir)/help_main.o $(rt_msg_eobjs) \
	$(pwr_eobj)/rt_io_user.o -L/usr/X11R6/lib\
	-L/opt/gnome/lib \
	-lpwr_xtt_gtk -lpwr_xtt -lpwr_xtt_gtk -lpwr_xtt -lpwr_ge_gtk -lpwr_ge -lpwr_rt \
	-lpwr_cow_gtk -lpwr_cow -lpwr_co -lpwr_flow_gtk -lpwr_flow -lpwr_glow_gtk -lpwr_glow \
	`pkg-config --libs gtk+-2.0` \
	-lpwr_msg_dummy \
	-ldb -lrpcsvc -lpthread -lm -lrt -lcrypt
endif
