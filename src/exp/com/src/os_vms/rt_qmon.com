$! 
$! Proview   $Id: rt_qmon.com,v 1.2 2005-09-01 14:57:49 claes Exp $
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
$! rt_qmon.com -- run the Qcom monitor
$!
$! This command file is to be executed by the detached process
$! that runs the Proview/R Qcom monitor.
$!
$ set noon
$!
$! Check where the log should go...
$!
$ node = f$edit(f$getsyi("nodename"), "lowercase")
$ prcnam = f$edit(f$getjpi("", "prcnam"), "lowercase")
$ output = f$trnlnm ("QMON_OUTPUT")
$ if output .nes. ""
$ then
$   write sys$output "-- ''prcnam': redirecting output to ''output'"
$   windownam = f$fao("!AS@!AS", prcnam, node)
$   iconnam = f$fao("!AS@!AS", prcnam, node)
$   crea/term/display='output'/noprocess/define=qmon_tty/insert-
	/window_attributes=(rows=48,title="''windownam'",icon="''iconnam'",-
	font="-*-Terminal-*-*-*--*-100-*-*-*-*-*-*")
$   define/nolog sys$output qmon_tty:
$   define/nolog sys$input qmon_tty:
$ endif
$!
$! Shall we run the debugger?
$!
$ if f$trnlnm ("QMON_DEBUG")
$ then
$	debug = "/debug"
$ else
$	debug = "/nodebug"
$ endif
$ set proc/prio=9
$ run pwr_exe:rt_qmon.exe 'debug'
$ exit
