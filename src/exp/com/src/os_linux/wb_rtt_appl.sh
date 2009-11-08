#!/bin/bash
#
# Proview   $Id: wb_rtt_appl.sh,v 1.4 2005-09-01 14:57:49 claes Exp $
# Copyright (C) 2005 SSAB Oxel�sund AB.
#
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as 
# published by the Free Software Foundation, either version 2 of 
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with the program, if not, write to the Free Software 
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#
# Link rtt application
#
#
name=$1
action=$2
is_rttsys=$3
opsys=$4
menuname=dtt_appl_${name}_m
applname=ra_rtt_${name}
exename=ra_rtt_${name} 
arname=ra_rtt_${name}.a
arname_pict=ra_rtt_${name}_pict.a


if [ $is_rttsys = "0" ]
then
# echo "Not rttsys"

  if [ $action = "1" ]
  then
#   echo "Link"
    if [ ! -e $pwrp_lib/${arname} ]
    then
#     echo "Creating archive $arname"
      ar rc $pwrp_lib/${arname}
    fi

    if [ ! -e $pwrp_lib/${arname_pict} ]
    then
#     echo "Creating archive $arname_pict"
      ar rc $pwrp_lib/${arname_pict}
    fi

#link option file exists and is not empty
  if [ -s $pwrp_exe/ra_rtt.opt ]; then
    ld_opt_tmp="`cat $pwrp_exe/ra_rtt.opt`"
    ld_opt="`eval echo $ld_opt_tmp`"
  else
    ld_opt=""
  fi



    cc=gcc
    cinc="-I$pwr_inc -I$pwrp_rttbld -I-"
    cflags="-DOS_LINUX=1 -DOS=linux -DHW_X86=1 -DHW=x86 -O3 -DGNU_SOURCE -DPWR_NDEBUG -D_REENTRANT"

    ${cc} -c -o $pwrp_obj/${menuname}.o \
        $pwrp_rttbld/${menuname}.c \
        ${cinc} ${cflags}

    ld=g++
    linkflags="-g -L/lib/thread -L$pwrp_lib -L$pwr_lib"

    ${ld} ${linkflags} -o $pwrp_exe/${exename} $pwrp_obj/${menuname}.o \
     $pwr_obj/dtt_rttsys.o $pwr_obj/rt_io_user.o \
     $pwrp_lib/${arname_pict} $pwrp_lib/${arname} \
     $ld_opt \
     -lpwr_dtt\
     -lpwr_rt -lpwr_co -lpwr_flow -lpwr_msg_dummy -lrpcsvc -lpthread -lm -lrt -lcrypt
  fi

  if [ $action = "2" ]
  then
#   echo "Compile"
    cc=gcc
    cinc="-I$pwr_inc -I$pwrp_rttbld -I$pwrp_inc -I$pwrp_cmn/common/inc -I-"
    cflags="-DOS_LINUX=1 -DOS=linux -DHW_X86=1 -DHW=x86 -O3 -DGNU_SOURCE -DPWR_NDEBUG -D_REENTRANT"

    ${cc} -c -o $pwrp_obj/${applname}.o \
        $pwrp_rtt/${applname}.c \
        ${cinc} ${cflags}
    ar rc $pwrp_lib/${arname} $pwrp_obj/${applname}.o 

  fi
fi








