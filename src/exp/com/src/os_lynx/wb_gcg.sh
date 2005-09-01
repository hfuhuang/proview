#! /bin/bash
#
# Proview   $Id: wb_gcg.sh,v 1.2 2005-09-01 14:57:49 claes Exp $
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
# wb_gcg.com -- compile and link PLC code
#
# This compiles a file generated by wb_gcg
# and inserts it in the plc library
# It is called from the wb_gcg.c module.
#

let gcg__success=0
let gcg__compileerrors=1
let gcg__linkerrors=2
let gcg__archiveerrors=3

CompileProcess()
{
  if $cc_cmd -o $pwrp_obj/${FileName}.o $pwrp_gc/${FileName}.gc
  then
    echo "-- Plc process compiled for $OsStr $say_debug $ObjectName" 
    gcg_status=$gcg__success
  else
    echo "** Plc process compiled with errors $OsStr $ObjectName" 
    gcg_status=$gcg__compileerrors
  fi
  if [ $Clean -eq 1 ]; then
    rm $pwrp_gc/${FileName}.gc
  fi
}

CompileProgram()
{
  if $cc_cmd -o $pwrp_obj/plc_m${FileName}.o $pwrp_gc/plc_m${FileName}.gc
  then
    echo "-- Plc plcpgm compiled for $OsStr $say_debug $ObjectName" 
    gcg_status=$gcg__success
  else
    echo "** Plc plcpgm compiled with errors $OsStr $ObjectName" 
    gcg_status=$gcg_compileerrors
  fi
  if [ $Clean -eq 1 ]; then
    rm $pwrp_gc/plc_m${FileName}.gc
  fi
}

CompileWindow()
{
  if $cc_cmd -o $pwrp_obj/plc_m${FileName}.o $pwrp_gc/plc_m${FileName}.gc
  then
    echo "-- Plc window compiled for $OsStr $say_debug $ObjectName" 
    gcg_status=$gcg__success
  else
    echo "** Plc window compiled with errors $OsStr $ObjectName" 
    gcg_status=$gcg__compileerrors
  fi
  if [ $Clean -eq 1 ]; then
    rm $pwrp_gc/plc_m${FileName}.gc
    rm $pwrp_gc/plc_dec${FileName}.gc
    rm $pwrp_gc/plc_r1r${FileName}.gc
    rm $pwrp_gc/plc_r2r${FileName}.gc
    rm $pwrp_gc/plc_ref${FileName}.gc
    rm $pwrp_gc/plc_cod${FileName}.gc
  fi
}

CompileRtNode()
{

# link option file exists and is not empty
  if [ -s $pwrp_rexe/rt_plc_wb.opt ]; then
    ld_opt_tmp="`cat $pwrp_rexe/rt_plc_wb.opt`"
    ld_opt="`eval echo $ld_opt_tmp`"
  else
    ld_opt="`eval echo $pwr_obj/rt_io_user.o`"
  fi 

  if g++ $link_debug -L/lib/thread -L$pwrp_lib -L$pwr_lib -o $pwrp_exe/$OutFile \
    $pwr_obj/rt_plc_process.o \
    $pwrp_obj/${FileName}.o \
    $Libs \
    $ld_opt \
    $pwr_obj/pwr_msg_rt.o $pwr_obj/pwr_msg_co.o \
    -lpwr_rt -lpwr_co -lrpc -lnetinet -llynx -lpwr_msg_dummy
  then
    echo "-- Plc program linked for $OsStr $say_linkdebug node $FileName" 
    gcg_status=$gcg__success
  else
    echo "** Plc program linked with errors $OsStr node $FileName" 
    gcg_status=$gcg__linkerrors
  fi
  if [ -n "$pwrp_rexe" ]; then
    cp $pwrp_exe/$OutFile $pwrp_rexe/
  fi
}

CompileLibrary()
{
  echo "-- Building archive for volume: $VolumeId" 
  if ar -rc $pwrp_lib/${PlcLib} `ls $pwrp_obj/plc_m${VolumeId}*.o`
  then
    echo "-- Archive built for volume: $VolumeId" 
    gcg_status=$gcg__success
  else
    echo "** Error building archive for volume: $VolumeId" 
    gcg_status=$gcg__archiveerrors
  fi
}

OsMaskToOpSys ()
{
  let BitM=$1
  let Idx=0
  let Val=1

  while [ $Val -lt $BitM ]; do
    let Val=$Val*2
    let Idx=$Idx+1
  done
  let OpSys=Idx+1
}
#
#
#
#  Main
#
#
#
# Arguments
#
let Debug="$1"		# 1 if debug, 0 i nodebug
let FileType="$2"	# the type of file: rtnode,
			# plc or window module
FileName="$3"		# the name of the file to be compiled, 
VolumeId="$3"	        # VolumeId for objects to be inserted
let OsMask="$4"		# pwr_mOpSys
OutFile="$5"		# the name of the generated file
PlcLib="$5"		# library for filetype Program and Windoow
Libs="$6"		# link libraries
ObjectName="$6"		# name of object
SystemName="$7"		# name of system
ProjectRoot="$8"	# project root	

let Clean=0
OsMaskToOpSys $OsMask # Convert Bitmask to index

#
# Local symbols
#


let OpSys__Low=0
let OpSys_PPC_LYNX=4
let OpSys_X86_LYNX=5
let OpSys__High=6

vOpSys="vax_eln,vax_vms,axp_vms,ppc_lynx,x86_lynx"

let FileType__Low=-1
let FileType_Process=0
let FileType_Program=1
let FileType_Window=2
let FileType_RtNode=3
let FileType_Library=4
let FileType__High=5
vFileType="Process,Program,Window,RtNode,Library"
local_setup="pwr_gcg_setup.sh"

if [ -e ${local_setup} ]; then
# echo "-- Local setup file used"
  source ${local_setup} ${ProjectRoot} ${SystemName}
fi


if [ $Debug -eq 1 ]; then
  cc_debug="-g"
  link_debug="-g"
  say_debug="with debug"
  say_linkdebug="with debug"
else
  cc_debug="-O3" 
  say_debug="optimized -O3" 
  say_linkdebug=""
fi

#
# Check OpSys
#
# How can we find out if this is a ppc or x86 host?
#
if [ $OpSys -ne $OpSys_PPC_LYNX ] && [ $OpSys -ne $OpSys_X86_LYNX ]; 
then
  echo "Unknown operating system: $OpSys"
  exit -1
fi

#
# Check FileType
#
if [ $FileType -le $FileType__Low ] || [ $FileType -ge $FileType__High ];
then
  echo "Unknown file type: $FileType"
  exit -1
fi

OsStr="`echo $vOpSys| cut -f $OpSys -d ,`"
let FileTypeIdx=$FileType+1

pwrp_gc="$pwrp_tmp"

# Suppress all warnings, -x
cc_cmd="gcc -c -x c -w $cc_debug -DOS_LYNX -I$pwr_inc -I$pwrp_inc -I$pwrp_tmp -I-"

FileTypeStr="`echo $vFileType| cut -f $FileTypeIdx -d ,`"

# Execute build command
Compile$FileTypeStr
exit $gcg_status
