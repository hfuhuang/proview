! 
!  Proview   $Id: wb_foe_msg.msg,v 1.10 2007-09-21 08:13:43 claes Exp $
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
!  wb_foe_msg.msg -- <short description>

        .facility       GOEN,240 /prefix = GOEN__ ! Graphic objects

success		<successful completion>					/succ
nopoint		<there is no such connectionpoint>			/error
badcongrap	<bad connection definition value>			/error
paramism	<mismatch in number of parameters>			/error
badmethod	<no such graphic method>				/error
badindex	<bad graphic index>					/error

        .facility       GRE,250 /prefix = GRE__ ! Graphic editor

success		<successful completion>					/succ
noregsel	<no region selected>					/error
objnotfound	<object not found>					/error
center		<object is not centered>				/error

        .facility       GSX,270 /prefix = GSX__ ! Gre syntax control

success		<successful completion>					/succ
nextnode	<compile next node>					/info
nextpar		<compile next par>					/info
nopoint		<no point found>					/error
specfound	<special output method found>				/info
classref	<reference parameter not found>				/error
refobj		<referenced object does not exist>			/error
refpar		<referenced parameter does not exist>			/error
notcon		<input not connected>					/warn
conoutput	<two outputs connected>					/error
noinputs	<no inputs detected>					/warn
inv		<inverted inputs not allowed, ignored>			/warn
refclass	<class of referenced object not allowed>		/error
refpartype	<referenced parameter type is not correct>		/error
noreset		<no grafcet reset object found in plc attributes>	/error
classreset	<bad reset object class, di, do or dv is allowed>	/error
noorder		<connected object is not an order, not grafcet standard> /warn
nostep		<connected object has to be a step>			/error
pinwind		<both object connected to condition pin and subwindow found> /error
nopinorwind	<no object connected to condtition pin and no subwindow found> /warn
gnotcon		<grafcet object not connected>				/error
nocorder	<no c attribute found in order object>			/error
nosorder	<no s attribute found in order object>			/error
badwind		<object not allowed in this window>			/error
noexpr		<no expression found>					/error
nosubwindow	<no subwindow found>					/warn
noscantime	<bad scantime in plc attributes>			/error
nonode		<unable to get node object>				/error
classerr	<error in class definition, syntaxcontrol aborted>	/fatal
cwinderr	<compile errors, no object file produced>		/fatal
cwindwarn	<compile warnings>					/warn
cwindsucc	<successfull compilation>				/info
swinderr	<syntax errors, no object file produced>		/fatal
swindwarn	<syntax warnings>					/warn
swindsucc	<successfull syntax control>				/info
nodeclass	<node parameter in plcpgm has to be of class rtnode>	/error
badscantime	<scantime has to be a multiple of 0.02 s>		/error
ordermism	<missmatch in orderattributes>				/error
orderattr	<unknown order attribute>				/error
apinwind	<both object connected to activity pin and subwindow found> /error
anopinorwind	<no object connected to activity pin and no subwindow found> /warn
already_inserted <object already inserted in list>			/warn
ambigous_executeorder <ambigous execute order>				/error
plcwind_errors	<plc window compile errors>				/error
plcpgm_errors	<plc program compile errors>				/error
nobackup	<unable to get backup parameter>			/error
exfreq		<number of PLC frequencies has exceeded its limit>	/warn
nosystemobj	<system object is not fount>				/error
notsaved	<session is not saved>					/error
openfile	<unable to open file>					/error
dataasyntax	<syntax error in dataarithm expression>			/error
filecreated	<file is created>					/info
contype		<connected attributes are not of the same type>		/error
dataabufsize	<intern buffersize to small for code conversion>	/error
nomemory	<unable to allocate dynamic memory>			/error
unknopsys	<unknown operating system in node object>		/fatal
noopsys		<unknown operating system in volume object>		/fatal
noloadfile	<Loadfile is not created>				/error
noplc		<No plcpgm found>					/info
nobuildhost	<cannot get build host name>				/error
nothread	<Invalid thread object>					/error
invthread	<Invalid thread object, plc will not be executed>	/warning
generic		<Generic object is not connected>			/error
templateplc	<Template plc not found>				/error
ccerror		<Error from c compilator, see terminal window>		/fatal
noconnect	<PlcConnect object is missing>				/error
refconnect      <Error in PlcConnect object class>			/error
conbackup	<Backup connected to invalid attribute>			/error
replaced	<Symbol replaced>					/info
nomodif		<Nothing is modified>					/info
compileparent	<Compile parent window>					/info
disabled	<Object is disabled>					/error

        .facility       FOE,280 /prefix = FOE__ ! Function object editor

success		<successful completion>					/succ
windexist	<window already exists>					/info
qualreq 	<qualifier requested>					/error
qualsynt	<bad qualifier syntax>					/error
nonodes		<no nodes found>					/info
nocons		<no connections found>					/info
plcnotfound 	<plcpgm not found>					/error
nowind		<no windows found>					/info
windnotfound 	<window not found>					/error
noobjfound	<no objects found>					/info
classqual	<class qualifier is missing>				/error
classynt 	<error in class syntax>					/error
classname	<error in class name>					/error
hiername	<error in hierarchy object name >			/error
parsynt		<error in parameter syntax>				/error
parelsynt	<error in parameter element syntax>			/error
object		<object does not exist>					/error
objdid		<objdid does not exist>					/error
nopar		<no such parameter>					/error
element		<element is not correct>				/error
setptr		<set of pointers is not allowed>			/error
nodename	<error in nodename>					/error
objname		<error in object name>					/error
class		<error in object class>					/error
noplc		<object has to be a plcpgm>				/error
nowild		<no name without wildcard in sublist description>	/error
channame	<channelname has to include a # sign>			/error
nomemory	<unable to allocate dynamic memory>			/error
plcloaded	<the plc is loaded though it should not be>		/error
abortsearch	<abort search operation>				/error
nofile		<unable to open file>					/error
listobject	<list object does not exist>				/error
maxitem		<max number of items in input list exceeded>		/error
objectvol	<object is not in the current volume>			/error
dbopen		<database is already open>				/error
volumeclass	<error in volume class>					/error
volumeid	<error in volume id>					/error
nodb		<no such database>					/error
sh		<error from shell>					/error
replaced	<object is replaced>					/info

        .facility       ROU,290 /prefix = ROU__ ! Routed connections

success		<successful completion>					/succ
calcerror	<error calulating breakpoints>				/error

        .facility       TRA,300 /prefix = TRA__ ! Trace

success 	<successful completion>				/succ
discard 	<this node is not tracable>			/info
alreadyinit 	<gdh init already done>				/info
nohinodes 	<no nodes to highlight>				/info
notloaded 	<vldh node not loaded>				/error
nosize 		<can't get the parameter size>			/error
badarray 	<ad parameter value for par of type array>	/error
badmethod 	<tracemethod out of range>			/error
nopar 		<no parameter found with debugpar for name>	/error
nortnodes 	<the initial list of rtnodes is empty>		/error
nortnode 	<there is no rtnode for this vldhnode>		/error
badrtcount 	<number of rtnodes is false>			/error
nofloat 	<The parameter is not a float>			/error
nonodes 	< no nodes to trace>				/error
badpartype 	< bad parameter type>				/error
badparindex 	< bad parameter index>				/error
noananode 	< no analyse node>				/error
nomethod 	< method no yet implemented>			/error
noana 		<there is no analyse node for this connection>	/error
twoana 		<there is two analyse nodes for this connection> /error
toomanycon 	<too many conn to an analyse node>		/error
badclass 	<connection not allowed to objects of this class>   /error
