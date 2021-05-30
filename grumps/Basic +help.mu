@create Help Commands <HC>=10
@create Help Functions <HF>=10
@create Help Data <HD>=10
@set HC=inherit
@set HF=halt
@set HD=halt
@parent HC=HF
@parent HF=HD
@force me=@vd HF=num(HD)

&fn-listtopics HF=sort(iter(lattr(%vD/help-*), first(v(itext(0)), |),, |),, |)

&fn-searchtopics HF=sort(setunion(setr(0, iter(grepi(%vD, help*-*, %0), first(v(help-[rest(itext(0), -)]), |),, |)), %q0, |),, |)

&fn-exacttopicmatches HF=setinter(ucstr(sort(iter(lattr(%vD/help-*), v(itext(0)),, |),, |)), ucstr(%0), |)

&fn-findtopic HF=if(t(setr(0, iter(grepi(%vD, help-*, *%0*), ucstr(iter(v(itext(0)), itext(0)~[rest(itext(1), -)], |, |)),, |))), if(t(setr(1, member(setr(2, iter(%q0, first(itext(0), ~), |, |)), ucstr(%0), |))), rest(extract(%q0, %q1, 1, |), ~), if(t(setr(1, match(%q2, %0*))), rest(extract(%q0, %q1, 1, |), ~), if(t(setr(1, match(%q2, *%0*))), rest(extract(%q0, %q1, 1, |), ~), #-1 NO MATCH))), #-1 NO MATCH)

&cmd-+help HC=$+help*:@switch/first %0=,{@pemit %#=[stdheader(Help)]%R[threecolumns(u(fn-listtopics), |)]%R[stdfooter(+help <topic> or +help/search <text> for more)];},/s*,{@pemit %#=These topics contain the text you referenced: [elist(u(fn-searchtopics, rest(%0)),, |)];},{@eval setq(E, if(not(t(setr(T, ulocal(fn-findtopic, trim(squish(%0)))))), Could not find that topic: %qT)); @switch %qE=,{@pemit %#=[stdheader(first(v(help-%qT), |), Help)]%R[u(helptopic-%qT)]%R[stdfooter(Also called: [elist(v(help-%qT),, |)])];},{@pemit %#=[u(prefix)] %qE;}}
@set HC/cmd-+help=no_parse

&cmd-+addhelp HC=$+addhelp *=*:@eval setq(E, if(switch(%0, *~*, 1, 0), You may not use ~ in your help file name.%b)[if(t(setr(M, u(fn-exacttopicmatches, %0))), The following terms are already used for finding other topics: [elist(%qM,, |)])]); @switch %qE=,{&help-[setr(C, add(default(%vD/helpcount, 0), 1))] %vD=%0; &helptopic-%qC %vD=%1; &helpcount %vD=%qC; @force %#=+help [first(%0, |)];},{@pemit %#=[u(prefix)] %qE;}
@set HC/cmd-+addhelp=no_parse
@set HC/cmd-+addhelp=uselock
&~cmd-+addhelp HC=[u(stafflock, %#)]

&cmd-+delhelp HC=$+delhelp *:@eval setq(E, if(not(t(setr(T, u(fn-findtopic, %0)))), Could not find that topic: %qT)); @switch %qE=,{@pemit %#=To undo, enter the following command:%R%R+addhelp [v(help-%qT)]=[v(helptopic-%qT)]%R%RYour topic has been deleted.; @wipe %vD/help*-%qT;},{@pemit %#=[u(prefix)] %qE;}
@set HC/cmd-+delhelp=no_parse
@set HC/cmd-+delhelp=uselock
&~cmd-+delhelp HC=[u(stafflock, %#)]

&cmd-+edithelp HC=$+edithelp */*=*/*:@eval setq(E, if(not(t(setr(T, u(fn-findtopic, %0)))), Could not find that topic: %qT, setq(F, switch(%1, n*,, topic)))); @switch %qE=,{&help%qF-%qT %vD=[edit(v(help%qF-%qT), %2, %3)]; @force %#=+help [first(%0, |)];},{@pemit %#=[u(prefix)] %qE;}
@set HC/cmd-+edithelp=no_parse
@set HC/cmd-+edithelp=uselock
&~cmd-+edithelp HC=[u(stafflock, %#)]



+addhelp Help|+help=%b Usage:%R%T+help - list all available topics in alphabetical order%R%T+help <topic> - find information about a topic.%R%T+help/s\[earch\] <term> - find all topics containing that term%R%R %bStaff-only commands:%R%T+addhelp <title>\[|<alternate title>\[|<etc>\]\]=<text> - add a new topic%R%T+delhelp <topic> - delete a topic%R%T+edithelp <topic>/(name|body)=<to>/<from> - edit a topic's name or body

