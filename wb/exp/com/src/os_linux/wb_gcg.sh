#! /bin/bash
#
# Proview   $Id: wb_gcg.sh,v 1.5 2007-11-26 13:11:32 claes Exp $
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
# wb_gcg.sh -- compile and link PLC code
#
# This compiles a file generated by wb_gcg
# and inserts it in the plc library
# It is called from the wb_gcg.c module.
#

let gcg__success=0
let gcg__compileerrors=1
let gcg__linkerrors=2
let gcg__archiveerror=3
let gcg__rsherror=4

#
# MyRsh returns the return status from the compile command, not from rsh 
# It only works on remote unix systems, not VMS...
#
MyRsh()
{
  hostname=
  lflag=
  nflag=
  user=

  case $1 in
   -l)
      ;;
   *)
        hostname=$1
        shift
  esac

  case $1 in
    -l)
	lflag=-l
	user=$2
	shift 2
  esac

  case $1 in
    -n)
	nflag=-n
	shift
  esac

  case $hostname in
    '')
	hostname=$1
	shift
  esac

  case $# in
    0)
	exec /usr/ucb/rlogin $lflag ${user+"$user"} "$hostname"
  esac

  AWK='
	NR > 1 {
		print prev;
		prev = $0;
		prev1 = $1;
		prev2 = $2;
	}
	NR == 1 {
		prev = $0;
		prev1 = $1;
		prev2 = $2;
	}
	END {
		if (prev1 ~ /[0-9]*[0-9]0/)
			exit(prev1 / 10);
		if (prev1 == "0")
			exit(prev2);
		print prev;
		exit(1);
	}
'

  exec 3>&1

  if rsh "$hostname" $lflag ${user+"$user"} $nflag \
	"(${*-:}); sh -c '"'echo "$0 $1" >&2'\'' $?0 "$status"' \
	2>&1 >&3 3>&- | awk "$AWK" >&2 3>&-
  then
    gcg_status=0
  else
    gcg_status=1
  fi
}


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
  if [ $Debug -eq 0 ]; then
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
    gcg_status=$gcg__compileerrors
  fi
  if [ $Debug -eq 0 ]; then
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
    echo "** Plc window compiled with errors for $OsStr $ObjectName" 
    gcg_status=$gcg__compileerrors
  fi
  if [ $Debug -eq 0 ]; then
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

#link option file exists and is not empty
  if [ -s $pwrp_exe/$FileName.opt ]; then

    ld_opt_tmp="`cat $pwrp_exe/$FileName.opt`"
    ld_opt="`eval echo $ld_opt_tmp`"
  else
    ld_opt="`eval echo $pwr_obj/rt_io_user.o -lpwr_usbio_dummy`"
  fi

  if g++ $link_debug -L/lib/thread -L$pwrp_lib -L$pwrp_cmn/x86_linux/lib -L$pwr_lib \
    -o $pwrp_exe/$OutFile \
    $pwr_obj/rt_plc_process.o \
    $pwrp_obj/${FileName}.o \
    $Libs \
    $ld_opt \
    $pwr_obj/pwr_msg_rt.o $pwr_obj/pwr_msg_co.o \
    -lrt -lpwr_remote -lpwr_nmps -lpwr_rt -lpwr_co -lrpcsvc -lpwr_msg_dummy -lpthread -lm -lusbio
  then
    echo "-- Plc program linked for $OsStr $say_linkdebug node $FileName" 
    gcg_status=$gcg__success
  else
    echo "** Plc program link errors for $OsStr node $FileName"
    gcg_status=$gcg__linkerrors
  fi
  if [ -n "$pwrp_exe_target" ]; then
    cp $pwrp_exe/$OutFile $pwrp_exe_target
    echo "-- Plc copied to $pwrp_exe_target"
  fi
}

CompileLibrary()
{
  echo "-- Building archive for volume: $VolumeId" 
  if ar -rc $pwrp_lib/$PlcLib `ls $pwrp_obj/plc_m${VolumeId}*.o`
  then
    echo "-- Archive built for volume: $VolumeId"
    gcg_status=$gcg__success
  else
    echo "** Error builing archive for volume: $VolumeId"
    gcg_status=$gcg__archiveerror
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

OsMaskToOpSys $OsMask # Convert Bitmask to index

#
# Local symbols
#


let OpSys__Low=0
let OpSys_VAX_ELN=1
let OpSys_VAX_VMS=2
let OpSys_AXP_VMS=3
let OpSys_PPC_LYNX=4
let OpSys_X86_LYNX=5
let OpSys_PPC_LINUX=6
let OpSys_X86_LINUX=7
let OpSys__High=8

vOpSys="vax_eln,vax_vms,axp_vms,ppc_lynx,x86_lynx,ppc_linux,x86_linux"

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
#  echo "-- Local setup file used"
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

if [ $OpSys -eq $OpSys_PPC_LINUX ] || [ $OpSys -eq $OpSys_X86_LINUX ]; 
then

  pwrp_gc="$pwrp_tmp"

# Suppress all warnings, -x
  cc_cmd="gcc -c -x c -w $cc_debug -D_REENTRANT -DOS_LINUX -I$pwr_inc -I$pwrp_inc -I$pwrp_tmp -I$pwrp_cmn/common/inc"

  FileTypeStr="`echo $vFileType| cut -f $FileTypeIdx -d ,`"

# Execute build command
  Compile$FileTypeStr
  exit $gcg_status

elif [ $OpSys -eq $OpSys_AXP_VMS ]; then

  rsh $pwr_build_host_axp_vms @pwr_exe:wb_gcg \"$1\" \"$2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \"$pwrp_root\"
  exit $?

elif [ $OpSys -eq $OpSys_VAX_VMS ]; then

  rsh $pwr_build_host_vax_vms @pwr_exe:wb_gcg \"$1\" \"$2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \"$pwrp_root\"
  exit $?

elif [ $OpSys -eq $OpSys_VAX_ELN ]; then

  rsh $pwr_build_host_vax_eln @pwr_exe:wb_gcg \"$1\" \"$2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \"$pwrp_root\"
  exit $?

elif [ $OpSys -eq $OpSys_X86_LYNX ]; then

  MyRsh $pwr_build_host_x86_lynx '$pwr_exe/wb_gcg.sh' \"$1\" \"$2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \"$pwrp_root\"
  exit $gcg_status

else
  echo "Unknown operating system: $OpSys"
  exit -1
fi










