! 
!  Proview   $Id: rt_rtt_msg.msg,v 1.2 2005-09-01 14:58:00 claes Exp $
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

        .facility       RTT,310 /prefix = RTT__ ! 

success 	<successful completion>				/succ
nopicture	<no picture drawn>				/info
fastback	<fast return to root menu>			/info
input_syntax	<input syntax error>				/error
stringtolong	<input string to long>				/error
objnotfound	<object not found>				/error
undef_pointer	<undefined rtdb pointer>			/error
com_ambig	<ambiguous command>				/error
com_nodef	<command not defined>				/error
qual_nodef	<qualifier not defined>				/error
qual_ambig	<ambiguous qualifier>				/error
qual_nofound	<qualifier not found>				/error
maxcount	<maxcount exceded>				/error
nocommand	<no command given>				/info
holdcommand	<keep command mode>				/info
item_nocrea	<no menu item created>				/info
nohelpsubj	<no help on this subject>			/error
nomemory	<unable to allocate dynamic memory>		/error
back		<return to previous menu>			/info
backtocollect	<return to collection menu>			/info
maxlimit	<maxlimit exceeded>				/error
minlimit	<minlimit exceeded>				/error
nobackground	<background is not drawn>			/info
picturenotsaved	<picture found that is not saved>		/error
nofile		<unable to open file>				/error
eoc		<end of command array>				/error
eof		<end of file>					/error
keynotdefined	<no picture defined on key>			/error
nopriv		<no privilege for attempted operation>		/error
novalue		<no value entered>				/info
cmdarg		<to many arguments in commandfile call>		/error
confabo		<action not confirmed>				/error
itemunknown	<unknown object i update item>			/error
redraw		<redraw the picture>				/info
symbol_ambig	<ambiguous command symbol>			/error
nosymbol	<symbol not defined>				/error
endstructerr	<syntax error at end of struct>			/error
elemsyntax	<syntax error in struct element>		/error
arraydim	<dimension of array not supported>		/error
structnotfound	<the struct was not found>			/error
