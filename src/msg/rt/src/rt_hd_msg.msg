! 
!  Proview   $Id: rt_hd_msg.msg,v 1.2 2005-09-01 14:58:00 claes Exp $
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
	.facility	HD,50 /prefix = HD__		! Historical data

success		<normal, successful completion>				/succ
nomessavail	<no messages are available> 				/warn
nosuchpamstype	<no such PAMS message type exists>			/warn
nosuchpamsclass	<no such PAMS message class exists>			/warn
pamsgetmsg 	<error get PAMS message>				/error
pamsrcvmsg 	<error receive PAMS message>				/error
pamsfreemsg 	<error dispose received PAMS message>			/error
freemsg 	<error dispose message>	 				/error
pamsallocmsg 	<no PAMS message area is allocated>			/error
nomesssend 	<no message is send>					/warn
pamsputmsg 	<error put PAMS message>				/error
pamssendmsg 	<error send PAMS message>				/error
errhd 	 	<error in hd>	 					/error
errinit 	<error initialize hd>	 				/error
getlocalobj 	<no local object of type '%s'>				/info
nosuchdesc	<no such descriptor name exists>			/error
errcredb	<error create Rtdb in '%s'>				/error
errgdhini	<initialize GDH in '%s'>				/error
errgetlist	<get type list '%s'>					/error
errgetpointer	<get pointer to object of type '%s'>			/error
errgetobjid	<get objid to object of type: '%s'>			/error
errgetnext	<get next object of type: '%s'>				/error
errcremess	<create message for: '%s'>				/error
errsendmess	<send message for: '%s'>				/error
errrecmess	<receive message>					/error
errwaitany	<wait any>						/error
errdelmess	<delete message>					/error
errascef	<associating event flag cluster, '%s'>			/error
alarmlstfull	<alarm list is full, max number of alarms/appl= %d>	/error
errlstlocate	<search for '%s' in '%s'>				/error
allrcon		<already connected to Message Handler>			/error
notconn		<not connected to Message Handler>			/info
nooutunit	<this is no Outunit>					/error
error		<handler error '%s'>					/error
log		<handler log '%s'>					/info
nospace		<no dynamic space available>				/error
applquota	<number of application alarms has exceeded its quota>	/error
badparam	<bad parameter value>					/error
noappl		<this is no Appl object>				/error
eventerr	<unexpected event>					/error
progerr		<programming error in hd>				/error
nyi		<not yet implemented>					/error
nonexalarm	<a non existing alarm>					/error
allret		<alarm already returned>				/info
notowned	<alarm not owned by application>			/error
version		<wrong version of HD-protocoll>				/error
nomoremsg	<no more messages owned by application>			/info
nosuchobj	<no such object exists>					/error
tablefull	<table '%s' full, point '%s' is not stored>		/info
tablenotfound	<table '%s' not found, point '%s' is not stored>	/info
pointlost	<point '%s' could not be stored>			/info
formaterror	<Point '%s' could not be stored because of bad format>	/info
sqlerr		<SQL Error when: %s , SQLCA.SQLCODE = %d>		/error
xdrfailed	<xdr failed>						/error
xdrunexpect	<xdr unexpected message type>				/error
insvirmem	<insufficient virtual memory>				/error
	.end
