@@ MUSH +msg Machine
@@ A simple alternative to +mail
@@ Version 1.5, November 24, 1993
@@ Runs under TinyMUSH version 2.0.9 p2.

@@ To Install:
@@
@@ Uncomment the @create lines below, and quote this file to the
@@ MUSH.  Put the Message Machine in your global room, and keep
@@ your Message Database somewhere handy.  The parents of objects
@@ in the master room are not checked for commands.

@@ @create Message Machine=10
@set Message Machine=INHERIT
@set Message Machine=SAFE

@@ @create Message Database=10
@set Message Database=SAFE
@set Message Database=HALT

@parent Message Machine=Message Database

@@ Commands:

@@ Syntax: +msg <recipient>[,<recipient2>,...]=<message>
@@         +msg
@@         +msg/check
@@         +msg/scan
@@         +msg/list [<player>]
@@         +msg/listall
@@
@@ The +msg command is a simple way to leave a message for someone
@@ who is currently not logged on the MUSH.  To send a message, use
@@ the first version as listed above: '+msg player=message'.
@@
@@ To see if you have any messages waiting, type '+msg/check' -- this
@@ is a command you will probably want to put in your @aconnect.  To
@@ read your messages, use the '+msg' command by itself.  Messages
@@ are usually cleared when read; to read without clearing, use the
@@ '+msg/scan' command instead.
@@
@@ '+msg/list' will list all of the messages you have waiting, and who
@@ they are from; a wizard may specify a player, or list all of the
@@ messages in the database.

&CMD-MSG Message Machine=$+msg *=*:@dolist/delimit , %0={@switch num(*##)=#-1*,{@pemit v(#)=No such player '[##]'.},{@tr me/sub-msg=num(*##),%1,v(edit(num(*##),#,max-)),%#}}
&CMD-MSG_CHECK Message Machine=$+msg/check:@switch add(0,v(edit(%#,#,max-)))=0,{@pemit %#=You have no messages waiting.},{@pemit %#=You have [setq(0,add(0,v(edit(%#,#,max-))))][r(0)] message[switch(r(0),1,,s)] waiting. Read with +msg.}
&CMD-MSG_LIST Message Machine=$+msg/list:@pemit %#=[u(fmt-listall,edit(%#,#,MAX-))]
&CMD-MSG_LISTALL Message Machine=$+msg/listall:@switch hasflag(%#,wizard)=0,{@pemit %#=Permission denied.},{@dolist lattr(me/max-*)={@pemit %#=[u(fmt-listall,##)]}}
&CMD-MSG_LIST_PLAYER Message Machine=$+msg/list *:@switch hasflag(%#,wizard)=0,{@pemit %#=Permission denied.},{@pemit %#=[u(fmt-listall,[edit(num(*%0),#,MAX-)])]}
&CMD-MSG_READ Message Machine=$+msg:@switch add(0,v(edit(%#,#,max-)))=0,{@pemit %#=You have no messages.},{@dolist lnum(v(edit(%#,#,max-)))={@pemit %#=v([edit(%#,#,msg-)]-[add(1,##)]);&[edit(%#,#,max-)] [parent(me)];&[edit(%#,#,msg-)]-[add(1,##)] [parent(me)]}}
&CMD-MSG_SCAN Message Machine=$+msg/scan:@switch add(0,v(edit(%#,#,max-)))=0,{@pemit %#=You have no messages.},{@dolist lnum(v(edit(%#,#,max-)))={@pemit %#=v([edit(%#,#,msg-)]-[add(1,##)])}}
&FMT-LISTALL Message Machine=[setq(0,edit(%0,MAX-,#))][ljust(name(r(0)),20)][setq(1,v(%0))][switch(add(r(1),0),0,No messages.,[r(1)] message[switch(r(1),1,,s)] waiting:[iter(lnum(r(1)),%r[space(20)][ljust(before([setq(2,v([edit(r(0),#,msg-)]-[add(##,1)]))][r(2)],%(),35)] ([extract(after(r(2),%)),2,3)]))])]
&SUB-MSG Message Database=&[edit(%0,#,msg-)]-[add(%2,1)] [parent(me)]=Message from [name(%3)](%3) at [time()]: %1;&[edit(%0,#,max-)] [parent(me)] = [add(%2,1)];@switch hasflag(%0,connect)=1,{@pemit %0=You have a new message. Read with +msg.};@pemit %3=Your message has been sent to [name(%0)].
