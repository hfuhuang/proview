$! 
$! Proview   $Id: rt_hd_store_start.com,v 1.2 2005-09-01 14:57:49 claes Exp $
$! Copyright (C) 2005 SSAB Oxel�sund AB.
$!
$! This program is free software; you can redistribute it and/or 
$! modify it under the terms of the GNU General Public License as 
$! published by the Free Software Foundation, either version 2 of 
$! the License, or (at your option) any later version.
$!
$! This program is distributed in the hope that it will be useful 
$! but WITHOUT ANY WARRANTY; without even the implied warranty of 
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
$! GNU General Public License for more details.
$!
$! You should have received a copy of the GNU General Public License 
$! along with the program, if not, write to the Free Software 
$! Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
$!
$! rt_hd_store_start.com
$!
$! Starts the store process the process will be the named
$! "* Store_groupnr *". It's running as a subprocess to adm
$!
$!
$! Check where the log should go...
$!
$ node = f$edit(f$getsyi("nodename"), "lowercase")
$ pwr_busid = "pwr_''node'_busid"
$ prcnam = "PWR_HDSTORE_''f$trnlnm(pwr_busid)'"
$ set process/name="''prcnam'"
$ prcnam = f$edit(f$getjpi("", "prcnam"), "lowercase")
$ output = f$trnlnm ("HDSTORE_OUTPUT")
$ if output .nes. ""
$ then
$   write sys$output "-- ''prcnam': redirecting output to ''output'"
$   windownam = f$fao("!AS@!AS", prcnam, node)
$   iconnam = f$fao("!AS@!AS", prcnam, node)
$   crea/term/display='output'/noprocess/define=hdstore_tty-
	/window_attributes=(rows=48,title="''windownam'",icon="''iconnam'",-
	font="-*-Terminal-*-*-*--*-100-*-*-*-*-*-*")
$   define/nolog sys$output hdstore_tty:
$   define/nolog sys$input hdstore_tty:
$ endif
$!
$ define/process sys$scratch pwrp_lis
$! define/process RDMS$DEBUG_FLAGS "T"
$! define/process RDMS$DEBUG_FLAGS_OUTPUT pwrp_lis:fetch1debug.lis
$ define/process RDMS$BIND_WORK_FILE pwrp_lis
$
$ set working_set/quota=8192/EXTENT=32063
$ store :== $pwr_exe:rt_hd_store.exe
$
$ set proc/prio=6
$ store "''p1'"
$
$ exit
