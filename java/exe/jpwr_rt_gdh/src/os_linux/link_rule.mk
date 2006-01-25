ifndef link_rule_mk
link_rule_mk := 1

link =	g++ -shared -DHW_X86 -DOS_LINUX \
	-L${pwr_elib} \
	-L${pwr_lib} \
	-L/usr/X11R6/lib \
	${pwre_broot}/${pwre_target}/bld/lib/co/*.o \
	$(rt_msg_eobjs) \
	${pwre_broot}/${pwre_target}/bld/lib/msg_dummy/msg_dummy_qcom.o \
	${pwre_broot}/${pwre_target}/bld/lib/msg_dummy/msg_dummy_op.o \
	${pwre_broot}/${pwre_target}/bld/lib/msg_dummy/msg_dummy_wb.o \
	${pwre_broot}/${pwre_target}/bld/lib/msg_dummy/msg_dummy_ge.o \
	${pwre_broot}/${pwre_target}/bld/lib/msg_dummy/msg_dummy_flow.o \
	${pwre_broot}/${pwre_target}/bld/lib/msg_dummy/msg_dummy_pwrp.o \
	${pwre_broot}/${pwre_target}/bld/lib/rt/*.o \
	${pwre_broot}/${pwre_target}/bld/lib/flow/*.o \
	${pwre_broot}/${pwre_target}/exp/obj/rt_io_user.o \
	${pwre_broot}/${pwre_target}/exp/obj/dtt_rttsys.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/io_ssab_dummy.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/rtt_dummy.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/jpwr_rt_qcom.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/jpwr_rt_Errh.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/jpwr_rt_rtsecurity.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/jpwr_rt_mh.o \
	${pwre_broot}/${pwre_target}/bld/exe/jpwr_rt_gdh/jpwr_rt_hist.o \
	${pwr_obj}/jpwr_rt_gdh.o \
	-o ${pwr_exe}/libjpwr_rt_gdh.so -lm -lpthread -lrt -lpwr_dtt \
	-L/opt/gnome/lib \
	-lImlib -lMrm -lXm -lXpm -lXt -lX11 -lXext -lXp\
        -lSM -lICE -ldb-4.0


endif










