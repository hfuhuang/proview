! 
!  Proview   $Id: rs_sutl_msg.msg,v 1.2 2005-09-01 14:58:00 claes Exp $
!  Copyright (C) 2005 SSAB Oxel�sund AB.
!
!  This program is free software; you can redistribute it and/or 
!  modify it under the terms of the GNU General Public License as 
!  published by the Free Software Foundation, either version 2 of 
!  the License, or (at your option) any later version.
!
!  This program is distributed in the hope that it will be useful 
!  but WITHOUT ANY WARRANTY; without even the implied warranty of 
!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
!  GNU General Public License for more details.
!
!  You should have received a copy of the GNU General Public License 
!  along with the program, if not, write to the Free Software 
!  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
!

        .facility       SUTL,320 /prefix = SUTL__ ! 

success 	<successful completion>				/succ
invlopnr	<ogiltigt l�pummer>				/error
nofile		<kan ej �ppna fil>				/error
wdgnotinit	<watchdog not initialized>			/error
stalldo		<stall do card is not found>			/error
stallstart	<stall plc switching pass start>		/info
stallend	<stall plc switching pass end>			/info
pams_dcl	<Unable to declare this job to PAMS>		/fatal
prcv_init	<Unable to initialze PRCV '%s'>			/fatal
prcv_started    <PRCV-job is running>		                /info
main_pams	<Error in mainprocess after PAMS-message receive>	/error
main_switch	<Error in mainprocess after clas switch>		/error
applexit	<rs_lat: Application exited: '%s', Reason: >					/error
latcon		<rs_lat: Lat connection failed, Reason: >		/error
latexit		<rs_lat has exited, Reason: >				/error
latconabo	<Rejected: '%s'>						/error
latc		<rs_lat: Lat service connected: '%s' '%s'>		/info
latappl		<rs_lat: Application started: '%s'>			/info
latarg		<rs_lat: Maximum number of argumens exceeded>		/error
applstart	<rs_lat: Unable to start applicaton: '%s', Reason: >	/error
latdiscast	<rs_lat: Disconnect AST received, circuit disconnected>	/error
termappl	<rs_term: Application started: '%s'>			/info
termmaxarg	<rs_term: Maximum number of argumens exceeded '%s'>		/error
termminarg	<rs_term: Too few arguments passed to RS_TERM>		/error
termapplstart	<rs_term: Unable to start applicaton: '%s', Reason: >	/error
termport	<rs_term: Unable to create port: '%s', Reason: >		/error
termwait	<rs_term: Wait exited abnormal: '%s', Reason: >		/error
termdelete	<rs_term: Could not delete port at restart: '%s', Reason: > /error
