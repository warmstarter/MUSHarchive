@@ The Tiny CG
@@ -
@@ Requirements:
@@ -
@@ The game is assumed to have installed an isstaff function. Here is a sample
@@ from SGP Globals which you can edit to your liking:
@@ @switch version()=TinyMUSH version 2.2.*,{&FN_ISSTAFF SGP - Global Functions=orflags%(%%0,Wi%)},MUX*,{&FN_ISSTAFF SGP - Global Functions=orflags%(%%0,WZw%)},PennMUSH*,{&FN_ISSTAFF SGP - Global Functions=orflags%(%%0,Wr%)},{&FN_ISSTAFF SGP - Global Functions=orflags%(%%0,iWaBg%)}
@@ -
@@ User is expected to have Anomaly Jobs installed if you want players to
@@ request approval. You'll need a bucket called APPS, which is included by
@@ default, and the standard TRIG_APPLY. You must set the DBref of your job
@@ database in the settings file below.
@@ -
@@ TinyCG goals:
@@ -
@@ 1. Small and expandable.
@@ 2. Works on MUX and Rhost at least.
@@ 3. Not restricted by game system.
@@ 4. Include a simple, flexible die roller.
@@ 5. Track various stats (like XP, WP, etc) with simple up-down stuff.
@@ 6. Rules can be set as to what's legal and what's not pre-chargen.
@@ 7. Simple approval process.


@create Settings Database <STDB>=10
@set STDB=HALT
@desc STDB=%R%TThis is the giant box of all the chargen objects and the setter object for some basic settings. Leave it alone unless you know what you're doing.%R

@@ This is the "Jobs Database" object from your Anomaly Jobs install. Set it
@@ correctly or stuff will break.
&jobs_database STDB=#50


@@ Each stat and validator will be grouped on the same object. Stats may have up
@@ to 8 attributes per stat; validators will only have 4 per validator. Since we
@@ are grouping these items together, the max possible (albeit unlikely) number
@@ of items per object should be considered; take the number below and multiply
@@ it by 8, and that's how many attributes you /could/ end up with on an object
@@ before a new one gets made. In reality the result will probably be far
@@ smaller, since there isn't much reason to use /all/ the attributes possible.
@@ To keep your DBs manageable, make sure to unset superfluous stats or
@@ validators.  For example, if you convert a stat from a number to a formula,
@@ make sure to remove the old min and max values. This setting is provided in
@@ case your game requires fewer attributes on an object.
&max_attributes STDB=1000


@@ If this game requires approval, set this setting to 1; if not, set it to 0.
@@ Games which do not require approval allow players to continually edit and
@@ update their stats via +cg. Games which do require players to use +advance
@@ to change their stats once they have been approved, and may restrict players
@@ from going in-character before they are approved. The default of this
@@ setting is 1.
&game_requires_approval STDB=1


@@ This is an option some games might use to allow players to build standardized
@@ PCs and go straight IC without having to pass staff inspection. By default it 
@@ is 0, false, but if you set it to 1, players will be able to +cg/approve
@@ themselves. They will still have to pass any validators you put in place for
@@ the approval to be successful.
&players_can_approve_themselves STDB=0


@@ If the game allows advancement, set this setting to 1; if not, set it to 0.
@@ Games which do not allow advancement at all do not permit their players to
@@ ever change their stats, except in special, staff-overridable circumstances.
@@ This is set to 1 by default.
&game_allows_advancement STDB=1


@@ This allows players to change their own stats post-approval without help from staff. By default it is set to 1 - players can change their own stats freely. They will still need to pass any validations put in place with the special tag "Advancement". If set to 0, players may still use the +advance command, but it will 
&players_can_advance_themselves STDB=1


@@ ----- Do not touch below this line unless you know what you're doing. ------

@@ Function object below. Much of the work should happen here.

@create StatDB Functions <SDBF>=10
@set SDBF=HALT INHERIT
@parent SDBF=STDB

@@ Locks

&fn-isstaff SDBF=isstaff(%0)

&fn-canusecg SDBF=and(andflags(%0, Pc), not(cor(hasflag(%0, guest), haspower(%0, guest))))

@@ Layout settings

&fn-stdheader SDBF=%b.[center(< %0 >, 75, -)].

&fn-stdfooter SDBF=%b.[if(t(%0), center(< %0 >, 75, -), repeat(-, 75))].

&fn-threecolumns SDBF=%b [edit(iter(%0, %b[ljust(mid(itext(0), 0, setr(0, if(eq(mod(inum(0), 3), 2), 24, 23))), %q0)][if(eq(mod(inum(0), 3), 0), if(neq(words(%0, if(t(%1), %1, %b)), inum(0)), %R %b), %b%b)], if(t(%1), %1, %b), @@), @@,)]

@@ Functions go here.

&fn-listuntagged SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), iter(lattr(itext(0)/stat_*), if(not(t(xget(itext(1), tags_[rest(itext(0), _)]))), setq(0, setunion(%q0, xget(itext(1), itext(0)), |)))))])%q0

&fn-listtags SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), iter(lattr(itext(0)/tags_*), setq(0, setunion(%q0, xget(itext(1), itext(0))))))])%q0

&fn-liststatsbytag SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), iter(lattr(itext(0)/tags_*), setq(0, setunion(%q0, iter(xget(itext(1), itext(0)), if(strmatch(itext(0), *%0*), itext(2)|[last(itext(1), _)]))))))])%q0

&fn-findstatbyname SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), iter(lattr(itext(0)/stat_*), setq(0, setunion(%q0, iter(xget(itext(1), itext(0)), if(strmatch(itext(0), %0*), itext(2)|[last(itext(1), _)]))))))])%q0

&fn-findstatbyexactname SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), iter(lattr(itext(0)/stat_*), setq(0, setunion(%q0, iter(xget(itext(1), itext(0)), if(strmatch(itext(0), %0), itext(2)|[last(itext(1), _)]))))))])%q0

&fn-findstatbyid SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), setq(0, if(hasattr(itext(0), stat_%0), itext(0)|%0)))])%q0

&fn-getstatname SDBF=null(setq(0,)[iter(lcon(%vP, OBJECT), setq(0, if(hasattr(itext(0), stat_%0), xget(itext(0), stat_%0))))])%q0

&fn-getstat SDBF=default(%0/_stat_[rest(ulocal(fn-findstatbyname, %1), |)], 0)

&fn-findvalidatorbyid SDBF=null(setq(0,)[iter(lcon(%vG, OBJECT), setq(0, if(hasattr(itext(0), validator_%0), itext(0)|%0)))])%q0

&fn-listuntaggedvalidators SDBF=null(setq(0,)[iter(lcon(%vG, OBJECT), iter(lattr(itext(0)/validator_*), if(not(t(xget(itext(1), tags_[setr(1, rest(itext(0), _))]))), setq(0, setunion(%q0, %q1. [default(itext(1)/desc_%q1, No description set.)], |)))))])%q0

&fn-listvalidatortags SDBF=null(setq(0,)[iter(lcon(%vG, OBJECT), iter(lattr(itext(0)/tags_*), setq(0, setunion(%q0, xget(itext(1), itext(0))))))])%q0

&fn-listvalidatorsbytag SDBF=null(setq(0,)[iter(lcon(%vG, OBJECT), iter(lattr(itext(0)/tags_*), setq(0, setunion(%q0, iter(xget(itext(1), itext(0)), if(strmatch(itext(0), *%0*), itext(2)|[last(itext(1), _)]))))))])%q0

&fn-listvalidatorsbystat SDBF=null(setq(0,)[iter(lcon(%vG, OBJECT), iter(lattr(itext(0)/stat_*), setq(0, setunion(%q0, iter(xget(itext(1), itext(0)), if(t(member(itext(0), %0)), itext(2)|[last(itext(1), _)]))))))])%q0

&fn-findvalidatorsbymessage SDBF=null(setq(0,)[iter(lcon(%vG, OBJECT), iter(lattr(itext(0)/message_*), setq(0, setunion(%q0, iter(xget(itext(1), itext(0)), if(strmatch(itext(0), *%0*), itext(2)|[last(itext(1), _)]))))))])%q0

&fn-matchvalue SDBF=null(setq(0,)[iter(revwords(%0, |, |), if(strmatch(itext(0), %1*), setq(0, itext(0))), |)])%q0


@@ Key data per stat:
@@ 	ID: Saves me from having to make each one have a unique name.
@@ 	Name: What players will reference by (for the most part).
@@ 	Data type:
@@ 		Boolean
@@ 		String
@@ 		Date
@@ 		Number
@@ 		List of numbers (ex: 1 3 5 - good for merits)
@@ 		List of strings (ex: Faith|Hope|Charity etc)
@@ 		Formula (ex: Defense = lowest of either Dex or Wits + Equipment bonus)
@@ 	MinValue: 0 by default, only applies to numbers.
@@ 	MaxValue: Unlimited by default, only applies to numbers.
@@ 	Values: Only needed for lists; lists the values the user can choose from.
@@ 	Formula: Automatically figured, can't be set.
@@ 	Tags: Games it's valid for, lists it should appear on, etc.
@@ 		Ex: Attribute; Finesse; and Mental are tags for "Wits".
@@ 	Notes: Any notes to show the player - for example, book and page reference.
@@ 	Locked: 0 or 1
@@ 		0: Users can set any value; they will still be warned if it isn't kosher.
@@ 		1: Users can only set valid values. Other values will error.
@@ -
@@ Data format:
@@ 	&stat_ID obj=Name
@@ 	&min_ID obj=1
@@ 	&max_ID obj=10
@@ 	&values_ID obj=1|2|3|4
@@ 	&formula_ID obj=add(min(getstat(%0, Dexterity), getstat(%0, Wits)), getequipbonus(%0, Defense))
@@ 	&tags_ID obj=Tag1 Tag2 Tag3
@@ 	&notes_ID obj=Notes
@@ 	&locked_ID obj=0
@@ -
@@ Validators:
@@ 	ID: Checks will be referenced by ID.
@@ 	Stat(s) the user must have for the check to be run.
@@ 	Actual check: formula.
@@ 	Failure message: Text to show the user if failed.
@@ 	Tags: Stat tags the user must have for the check to be run.
@@ 	Desc: Validator's description for sanity's sake.
@@ 	Data passed in to the check:
@@ 		%0: The dbref of the PC to check.
@@ 	Data output by the check:
@@ 		0: Plain ol' failure.
@@ 		1: Success!
@@ 		#-1 Message: an error message.
@@ 		Anything else: Success!
@@  Data passed in to the message:
@@    %0: The dbref of the PC who was checked.
@@    %1: The result (can include #-1 ERROR messages) of the check.
@@ -
@@ Data format:
@@ 	&validator_ID obj=Check formula
@@ 	&stat_ID obj=1 14 39 2
@@ 	&message_ID obj=You have the merit "Blah" which requires a "Bloo" of 2.
@@ 	&tags_ID obj=Tag1 Tag2 Tag3
@@ 	&desc_ID obj=This tests whether the user has a valid blah.

@create Stat Database <SDB>=10
@set Stat Database <SDB>=HALT
@parent Stat Database <SDB>=SDBF
@desc Stat Database <SDB>=%R%TThis is the giant box of stat objects. It is automatically managed by code. You should not need to muck with this.%R%TThe current stat object's dbref is [v(currentDB)].%R
&vC Stat Database <SDB>=0

@create StatDB1 <SDB1>=10
@set SDB1=HALT
@parent SDB1=Stat Database <SDB>

@force me=&currentDB Stat Database <SDB>=num(SDB1)
@force me=@tel SDB1=Stat Database <SDB>

@force me=&vP STDB=num(Stat Database <SDB>)
&vS STDB=xget(%vP, currentDB)
&vR STDB=xget(%vG, currentDB)
@force me=@tel Stat Database <SDB>=STDB

@create Validator Database <VDB>=10
@set Validator Database <VDB>=HALT
@parent Validator Database <VDB>=SDBF
@desc Validator Database <VDB>=%R%TThis is the giant box of chargen validators. It is automatically managed by code. You should not need to muck with this.%R%TThe current check object's dbref is [v(currentDB)].%R
&vC Validator Database <VDB>=0

@create ValidatorDB1 <VDB1>=10
@set VDB1=HALT
@parent VDB1=Validator Database <VDB>
@force me=&currentDB Validator Database <VDB>=num(VDB1)
@force me=&vG STDB=num(Validator Database <VDB>)

@force me=@tel VDB1=Validator Database <VDB>
@force me=@tel Validator Database <VDB>=STDB

@@ Actual commands, OMG!

@create Chargen Commands <CGC>=10
@set CGC=INHERIT
@parent CGC=SDBF
@desc CGC=%R%TThis object offers commands for going through character generation. Here's a list of all currently available commands:%R%R[ulocal(fn-threecolumns, iter(sort(lattr(me/cmd-*)), rest(itext(0), -)))]
@lock CGC=fn-canusecg/1
@fail CGC=You are not currently permitted to use the character generator. This may be because you're a guest character, or because staff would like you to talk to them before you enter character generation. See the news files or talk to staff to find out how to get set up to use CG!

&cmd-+cg_set CGC=$+cg *=*:@switch/first setr(E, if(hasattr(%#, _approved), You may not set stats via +cg once you are approved. Use +advance instead!, if(not(t(%0)), You must enter a stat to set., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), Could not find the stat '%0'.)[setq(D, u(vS))])))=, {@switch %1=, {@wipe %#/_stat_[rest(%qS, |)];}, {@switch/first setq(P, first(%qS, |))[setq(N, xget(%qP, stat_[setr(I, rest(%qS, |))]))][setq(M, xget(%qP, min_%qI))][setq(X, xget(%qP, max_%qI))][setq(V, xget(%qP, values_%qI))][setq(L, xget(%qP, locked_%qI))][setq(F, xget(%qP, formula_%qI))][setr(E, if(t(%qF), You may not set the value of a stat which is determined by a formula.%b)[if(or(and(not(isnum(%1)), or(t(%qM), t(%qX))), and(isnum(%1), or(t(%qM), t(%qX)), or(gt(%1, %qX), lt(%1, %qM)))), The value you wish to set must be numeric and [if(and(t(%qM), t(%qX)), between %qM and %qX, if(t(%qM), greater than %qM, less than %qX))].%b)][if(and(t(%qV)[setq(O, ulocal(fn-matchvalue, %qV, %1))], t(%qL), not(t(%qO))), The stat '%qN' requires a value from the following list:%R%T[iter(%qV, itext(0), |, %R%T)])])]=, {&_stat_[rest(%qS, |)] %#=setr(F, if(t(%qO), %qO, %1)); @pemit %#=The stat '%qN' has been set to '%qF'.[if(and(t(%qV), not(t(%qO))), %bThis is a non-standard value for this stat%, which may require extra checking during approval. This stat is set up to allow any text you like%, as well as values from the following list:%R%T[iter(%qV, itext(0), |, %R%T)])];}, {@pemit %#=ERROR > %qE;}}}, {@pemit %#=ERROR > %qE;}

&cmd-+cg/approve CGC=$+cg/approve *:@switch/first setr(E, if(not(v(game_requires_approval)), This game does not require approval., if(not(t(setr(P, pmatch(case(%0, me, %#, *%0))))), Could not find the player '%0'., setq(N, name(%qP))[if(haspower(%qP, guest), You may not approve guests!, if(hasattr(%qP, _approved), %qN is already approved., if(and(not(isstaff(%#)), not(strmatch(%qP, %#))), You may only run this command on yourself. Try '+cg/approve me'.)))])))=, {&_validatorErrors %qP=0; @switch/first setq(A, setq(B, setq(V, )))[null(iter(lattr(%qP/_stat-*), setq(B, setunion(%qB, xget(setr(O, first(setr(I, u(fn-findstatbyid, rest(itext(0), _))), |)), tags_[setr(I, rest(itext(0), _))])))[setq(A, setunion(%qA, %qI))]))][null(iter(lcon(%vG, OBJECT), iter(lattr(itext(0)/validator_*), if(or(not(t(member(setr(T, xget(itext(1), tags_[setr(I, rest(itext(0), _))])), Advancement))), not(v(game_allows_advancement))), if(or(and(t(%qT), t(setinter(%qT, %qB))), and(t(setr(S, xget(itext(1), stat_%qI))), t(setinter(%qS, %qA))), and(not(t(%qT)), not(t(%qS)))), setq(V, setunion(%qV, itext(1)|%qI)))))))]%qV=, {}, {@dolist/notify %qV={@pemit %#=if(not(t(setr(E, ulocal(setr(O, first(##, |))/validator_[setr(I, rest(##, |))], %qP)))), udefault(%qO/message_%qI, Validator failed: %qE, %qP, %qE)); &_validatorErrors %qP=add(xget(%qP, _validatorErrors), 1);};}; @wait me={@pemit %#=GAME: Ran [words(%qV)] validators, found [setr(E, default(%qP/_validatorErrors, 0))] errors.; @switch/first %qE=0, {@switch and(not(v(players_can_approve_themselves)), not(isstaff(%#)))=1, {@trigger v(jobs_database)/TRIG_APPLY=%#;}, {&_approved %qP=[secs()] %# %N; @switch/first %qP=%#, {@pemit %#=You are now approved! Have fun!;}, {@pemit %#=You approved %qN!; @switch/first conn(%qP)=-1, {@mail/quick %qP/Congratulations!=You have been approved by %N. Have fun!;}, {@pemit %qP=You have been approved by %N!;};};};}, {@switch/first %qP=%#, {@pemit %#=You could not be approved due to validation errors. Please correct these issues and try again!;}, {@pemit %#=%qN could not be approved due to validation errors. Please correct these issues and try again.;};};};}, {@pemit %#=ERROR > %qE;}

&cmd-+cg/unapprove CGC=$+cg/unapprove *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to unapprove players., if(not(t(setr(P, pmatch(case(%0, me, %#, *%0))))), Could not find the player '%0'., setq(N, name(%qP))[if(haspower(%qP, guest), You may not unapprove guests!, if(not(hasattr(%qP, _approved)), %qN is not currently approved.))])))=, {@wipe %qP/_approved; @pemit %#=You unapproved %qN.; @switch/first conn(%qP)=-1, {@mail/quick %qP/Unapproved=You have been unapproved from roleplay by %N. Contact staff if you have any questions!;}, {@pemit %qP=You have been unapproved from roleplay by %N. Contact staff if you have any questions.;};}, {@pemit %#=ERROR > %qE;}

&cmd-+cg/override CGC=$+cg/override *:@switch/first setr(E, if(not(v(game_requires_approval)), This game does not require approval., if(not(isstaff(%#)), You must be staff to forcibly approve players., if(not(t(setr(P, pmatch(case(%0, me, %#, *%0))))), Could not find the player '%0'., setq(N, name(%qP))[if(haspower(%qP, guest), You may not approve guests!, if(hasattr(%qP, _approved), %qN is already approved.))]))))=, {&_approved %qP=[secs()] %# %N; @switch/first %qP=%#, {@pemit %#=You are now approved and ready to go IC! Have fun!;}, {@pemit %#=You approved %qN!; @switch/first conn(%qP)=-1, {@mail/quick %qP/Congratulations!=You have been approved for roleplay by %N. Get out there and have fun!;}, {@pemit %qP=You have been approved by %N!;};};}, {@pemit %#=ERROR > %qE;}



@create StatDB Commands <SDBC>=10
@set SDBC=INHERIT
@parent SDBC=SDBF
@desc SDBC=%R%TThis object offers commands for managing, maintaining, and reading the database of stats. Here's a list of all currently available commands:%R%R[ulocal(fn-threecolumns, iter(sort(lattr(me/cmd-*)), rest(itext(0), -)))]

&cmd-+stat/list SDBC=$+stat/list*:@switch/first trim(%0)=,{@pemit %#=[ulocal(fn-stdheader, All stat tags)]%R[ulocal(fn-threecolumns, ulocal(fn-listtags))]%R[ulocal(fn-stdfooter, +stat/list <tags> for more.)];}, untagged, {@pemit %#=[ulocal(fn-stdheader, Stats with no tags)]%R[ulocal(fn-threecolumns, ulocal(fn-listuntagged), |)]%R[ulocal(fn-stdfooter, +stat/info <stat> for more.)];}, {@pemit %#=[ulocal(fn-stdheader, Stats matching '[trim(%0)]')]%R[setq(S,)][iter(%0, setq(R, ulocal(fn-liststatsbytag, itext(0)))[if(t(%qS), setq(S, setinter(%qS, %qR)), setq(S, %qR))])][ulocal(fn-threecolumns, iter(%qS, xget(first(itext(0), |), stat_[rest(itext(0), |)]), , |), |)]%R[ulocal(fn-stdfooter, +stat/info <stat> for more.)];}

&cmd-+stat/info SDBC=$+stat/info *:@switch/first setr(E, if(not(t(%0)), You must enter a stat to get info on., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), No stat by that name appears to exist in the database., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(T, xget(%qD, tags_%qI))])))=, {@pemit %#=[ulocal(fn-stdheader, Stat Info: %qN)][if(isstaff(%#), %R %bID: %qI%R %bDB object: %qD)]%R %bName: %qN%R[if(hasattr(%qD, min_%qI), %b Minimum value: [xget(%qD, min_%qI)]%R)][if(hasattr(%qD, max_%qI), %b Maximum value: [xget(%qD, max_%qI)]%R)][if(hasattr(%qD, values_%qI), %b Value list: %R[ulocal(fn-threecolumns, xget(%qD, values_%qI), |)]%R)][if(hasattr(%qD, formula_%qI), %b Formula: [xget(%qD, formula_%qI)]%R)][if(hasattr(%qD, locked_%qI), %b Locked to values list: [if(t(xget(%qD, locked_%qI)), Yes, No)]%R)] %bTags: [if(t(%qT), itemize(%qT), No tags set yet.)]%R[ulocal(fn-stdfooter, Notes)]%R %b[edefault(%qD/notes_%qI, No notes set yet.)]%R[ulocal(fn-stdfooter)];}, {@pemit %#=ERROR > %qE;}
+stat/info Dex

&cmd-+stat/add SDBC=$+stat/add *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to add stats., if(not(t(%0)), You must enter a stat to add.)))=, {@switch/first setr(S, ulocal(fn-findstatbyexactname, %0))[setq(D, u(vS))]=, {@switch/first gt(attrcnt(%qD), v(max_attributes))=1, {@create StatDB[setr(B, words(lcon(%vP, OBJECT)))] <SDB%qB>=10; @parent [setr(D, num(StatDB%qB <SDB%qB>))]=%vP; @set %qD=HALT; &currentDB %vP=%qD; @tel %qD=%vP;}; &stat_[setr(C, add(xget(%vP, vC), 1))] %qD=%0; &vC %vP=%qC; @pemit %#=The stat '%0' has been created.;}, {@pemit %#=A stat named %0 already exists. For more information, try +stat/info %0.}}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/remove SDBC=$+stat/remove *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to remove stats., if(not(t(%0)), You must enter a stat to remove., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to remove., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))]))))=, {@pemit %#=Stat decompile:%R%R&stat_%qI %qD=%qN%R[if(hasattr(%qD, tags_%qI), &tags_%qI %qD=[xget(%qD, tags_%qI)]%R)][if(hasattr(%qD, notes_%qI), &notes_%qI %qD=[xget(%qD, notes_%qI)]%R)][if(hasattr(%qD, min_%qI), &min_%qI %qD=[xget(%qD, min_%qI)]%R)][if(hasattr(%qD, max_%qI), &max_%qI %qD=[xget(%qD, max_%qI)]%R)][if(hasattr(%qD, values_%qI), &values_%qI %qD=[xget(%qD, values_%qI)]%R)][if(hasattr(%qD, formula_%qI), &formula_%qI %qD=[xget(%qD, formula_%qI)]%R)][if(hasattr(%qD, locked_%qI), &locked_%qI %qD=[xget(%qD, locked_%qI)]%R)]; @wait 5={@wipe %qD/*_%qI; @pemit %#=Stat '%qN' deleted. If you want to restore it for the players who already have it, you MUST use the decompile - just copy and paste it. If you re-add it as a new stat, the players who already had it will have to reacquire it through chargen or XP-spending.}}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/name SDBC=$+stat/name *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to set the name of., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to set the name of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))]))))=, {&stat_%qI %qD=%1; @pemit %#=The stat '%qN' now has the name: %1;}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/tag SDBC=$+stat/tag *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to tag., if(not(t(%1)), You must enter one or more tags to add to the stat. Tags are separated by spaces., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to tag., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(T, xget(%qD, tags_%qI))], if(member(%1, Advancement), The tag 'Advancement' is reserved for the use of the +advance command and cannot be added to a stat.))))))=, {&tags_%qI %qD=setr(T, setunion(%qT, %1)); @pemit %#=The stat '%qN' has been tagged with '%1'. Its tags are now:%R%T[iter(%qT, itext(0), , %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/untag SDBC=$+stat/untag *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to untag., if(not(t(%1)), You must enter one or more tags to remove from the stat. Tags are separated by spaces., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to untag., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(T, xget(%qD, tags_%qI))][if(strmatch(setdiff(%qT, %1), %qT), The tag or tags '%1' were not found in the stat '%qN'.)])))))=, {&tags_%qI %qD=setr(T, setdiff(%qT, %1)); @pemit %#=The stat '%qN' has lost the tag '%1'. Its tags are now:%R%T[iter(%qT, itext(0), , %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/lock SDBC=$+stat/lock *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to lock., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to lock., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))]))))=, {&locked_%qI %qD=1; @pemit %#=The stat '%qN' has been locked.;}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/unlock SDBC=$+stat/unlock *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to unlock., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to unlock., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))]))))=, {&locked_%qI %qD=0; @pemit %#=The stat '%qN' has been unlocked.;}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/note SDBC=$+stat/note *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to set the notes of., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to set the notes of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))]))))=, {&notes_%qI %qD=%1; @pemit %#=The stat '%qN' now has the note:%R%1;}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/min SDBC=$+stat/min *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to set the minimum value of., if(and(not(isnum(%1)), not(strmatch(%1, ))), You must enter a numeric minimum value., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to set the minimum value of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(X, xget(%qD, max_%qI))][if(and(not(strmatch(%1, )), t(%qX), gt(%1, %qX)), The stat '%qN' has a maximum value of '%qX'. The value you entered%, '%1'%, is greater than this value. Please correct it.)])))))=, {&min_%qI %qD=%1; @pemit %#=The stat '%qN' has been set with a minimum value of '%1'.;}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/max SDBC=$+stat/max *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to set the maximum value of., if(and(not(isnum(%1)), not(strmatch(%1, ))), You must enter a numeric maximum value., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to set the maximum value of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(M, xget(%qD, min_%qI))][if(and(not(strmatch(%1, )), t(%qM), lt(%1, %qM)), The stat '%qN' has a minimum value of '%qM'. The value you entered%, '%1'%, is less than this value. Please correct it.)])))))=, {&max_%qI %qD=%1; @pemit %#=The stat '%qN' has been set with a maximum value of '%1'.;}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/val SDBC=$+stat/val *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to set the values of., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to set the values of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(L, xget(%qD, locked_%qI))][setq(V, xget(%qD, values_%qI))]))))=, {&values_%qI %qD=%1; @pemit %#=The stat '%qN' now has the following [if(t(%qL), permitted, recommended)] values:%R%T[iter(%1, itext(0), |, %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/addval SDBC=$+stat/addval *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to add a value to., if(strmatch(%1, ), You must enter a value to add., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to add a value to., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(L, xget(%qD, locked_%qI))][setq(V, xget(%qD, values_%qI))])))))=, {&values_%qI %qD=setr(V, setunion(%1, %qV, |)); @pemit %#=The stat '%qN' has a new value, '%1'. Its [if(t(%qL), permitted, recommended)] values are now:%R%T[iter(%qV, itext(0), |, %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/remval SDBC=$+stat/remval *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to remove a value from., if(strmatch(%1, ), You must enter a value to remove., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to add a value to., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))][setq(L, xget(%qD, locked_%qI))][setq(V, xget(%qD, values_%qI))][if(strmatch(setdiff(%qV, %1, |), %qV), You must choose an already-existing value to remove. Current values for '%qN' are:%R%T[iter(%qV, itext(0), |, %R%T)])])))))=, {&values_%qI %qD=setr(V, setdiff(%qV, %1, |)); @pemit %#=The stat '%qN' now has the following [if(t(%qL), permitted, recommended)] values:%R%T[iter(%qV, itext(0), |, %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+stat/formula SDBC=$+stat/formula *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit stats., if(not(t(%0)), You must enter a stat to set the formula of., if(not(t(setr(S, ulocal(fn-findstatbyname, %0)))), You must choose an already-existing stat to set the formula of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))]))))=, {&formula_%qI %qD=%1; @pemit %#=The stat '%qN' now has the following formula: [xget(%qD, formula_%qI)];}, {@pemit %#=ERROR > %qE;}
@set SDBC/cmd-+stat/formula=no_parse

@create ValidatorDB Commands <VDBC>=10
@set VDBC=INHERIT
@parent VDBC=SDBF
@desc VDBC=%R%TThis object offers commands for managing, maintaining, and reading the database of validators for character generation. Here's a list of all currently available commands:%R%R[ulocal(fn-threecolumns, iter(sort(lattr(me/cmd-*)), rest(itext(0), -)))]

&cmd-+validator/list VDBC=$+validator/list*:@switch/first trim(%0)=,{@pemit %#=[ulocal(fn-stdheader, All validator tags)]%R[ulocal(fn-threecolumns, ulocal(fn-listvalidatortags))]%R[ulocal(fn-stdfooter, +validator/list <tags> for more.)];}, untagged, {@pemit %#=[ulocal(fn-stdheader, Validators with no tags)]%R[iter(ulocal(fn-listuntaggedvalidators), %b [itext(0)], |, %R)]%R[ulocal(fn-stdfooter, +validator/info <ID> for more.)];}, {@pemit %#=[ulocal(fn-stdheader, Validators with tags matching '[trim(%0)]')]%R[setq(S,)][iter(%0, setq(R, ulocal(fn-listvalidatorsbytag, itext(0)))[if(t(%qS), setq(S, setinter(%qS, %qR)), setq(S, %qR))])][iter(%qS, rest(itext(0), |). [xget(first(itext(0), |), message_[rest(itext(0), |)])], , %R)]%R[ulocal(fn-stdfooter, +validator/info <ID> for more.)];}
+validator/list
+validator/list Attribute
+validator/list untagged

&cmd-+validator/find VDBC=$+validator/find *=*:@switch/first %0=s*,{@switch/first setr(E, if(not(t(%1)), You must enter a stat to get info on., if(not(t(setr(S, ulocal(fn-findstatbyname, %1)))), No stat by that name appears to exist in the database., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(N, xget(%qD, stat_%qI))])))=, {@pemit %#=[ulocal(fn-stdheader, Validators for stats matching '%qN')]%R[iter(ulocal(fn-listvalidatorsbystat, %qI), rest(itext(0), |). [default(first(itext(0), |)/message_[rest(itext(0), |)], No message.)], , %R)]%R[ulocal(fn-stdfooter, +validator/info <ID> for more.)];}, {@pemit %#=ERROR > %qE;}}, t*, {@pemit %#=[ulocal(fn-stdheader, Validators for tags matching '[trim(%1)]')]%R[setq(S,)][iter(%0, setq(R, ulocal(fn-listvalidatorsbytag, itext(0)))[if(t(%qS), setq(S, setinter(%qS, %qR)), setq(S, %qR))])][iter(%qS, rest(itext(0), |). [default(first(itext(0), |)/message_[rest(itext(0), |)], No message.)], , %R)]%R[ulocal(fn-stdfooter, +validator/info <ID> for more.)];}, m*, {@pemit %#=[ulocal(fn-stdheader, Validators with messages matching '[trim(%1)]')]%R[iter(ulocal(fn-findvalidatorsbymessage, %1), rest(itext(0), |). [xget(first(itext(0), |), message_[rest(itext(0), |)])], , %R)]%R[ulocal(fn-stdfooter, +validator/info <ID> for more.)];}, {@pemit %#=ERROR > Syntax is +validator/find stat=<stat name>, +validator/find tag=<tag name>, or +validator/find message=<partial or complete message text>.;}
+validator/find Stat=<stat name>
+validator/find Tag=<tag>
+validator/find Message=<message>


&cmd-+validator/info VDBC=$+validator/info *:@switch/first setr(E, if(not(t(%0)), You must enter a validator ID to get info on., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), No validator by that ID appears to exist in the database., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(M, xget(%qD, message_%qI))][setq(T, xget(%qD, tags_%qI))])))=, {@pemit %#=[ulocal(fn-stdheader, Validator Info: %0)][if(isstaff(%#), %R %bID: %qI%R %bDB object: %qD)]%R %bMessage: %qM%R[if(hasattr(%qD, stat_%qI), %b Runs for these stats: [itemize(iter(xget(%qD, stat_%qI), ulocal(fn-getstatname, itext(0)), , |), |)]%R)][if(hasattr(%qD, tags_%qI), %b Runs for these tags: [itemize(iter(xget(%qD, tags_%qI), itext(0), , |), |)]%R)]%R[ulocal(fn-stdfooter, Code)]%R %b[xget(%qD, validator_%qI)]%R[ulocal(fn-stdfooter)];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/add VDBC=$+validator/add *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to add validators., if(not(t(%0)), You must enter a validator to add.)))=, {@switch/first gt(attrcnt(setr(D, u(vR))), v(max_attributes))=1, {@create ValidatorDB[setr(B, words(lcon(%vG, OBJECT)))] <VDB%qB>=10; @parent [setr(D, num(ValidatorDB%qB <VDB%qB>))]=%vG; @set %qD=HALT; &currentDB %vG=%qD; @tel %qD=%vG;}; &validator_[setr(C, add(xget(%vG, vC), 1))] %qD=%0; &vC %vG=%qC; @pemit %#=A new validator with ID '%qC' has been created. Its code is:%R[xget(%qD, validator_%qC)];}, {@pemit %#=ERROR > %qE;}
@set VDBC/cmd-+validator/add=no_parse

&cmd-+validator/remove VDBC=$+validator/remove *:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to remove validators., if(not(t(%0)), You must enter a validator to remove., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), Could not find a validator with that ID., setq(I, rest(%qS, |))[setq(D, first(%qS, |))]))))=, {@pemit %#=Validator decompile:%R%R&validator_%qI %qD=[xget(%qD, validator_%qI)]%R[if(hasattr(%qD, tags_%qI), &tags_%qI %qD=[xget(%qD, tags_%qI)]%R)][if(hasattr(%qD, stat_%qI), &stat_%qI %qD=[xget(%qD, stat_%qI)]%R)][if(hasattr(%qD, message_%qI), &message_%qI %qD=[xget(%qD, message_%qI)]%R)]; @wait 5={@wipe %qD/*_%qI; @pemit %#=Validator '%0' deleted.}}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/set VDBC=$+validator/set *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to set the code of., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to set the code of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))]))))=, {&validator_%qI %qD=%1; @pemit %#=The validator '%0' now has the code:%R[xget(%qD, validator_%qI)];}, {@pemit %#=ERROR > %qE;}
@set VDBC/cmd-+validator/set=no_parse

&cmd-+validator/desc VDBC=$+validator/desc *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to set the desc of., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to set the code of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))]))))=, {&desc_%qI %qD=%1; @pemit %#=The validator '%0' now has the description: [xget(%qD, desc_%qI)];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/message VDBC=$+validator/message *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to set the message of., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to set the message of., setq(I, rest(%qS, |))[setq(D, first(%qS, |))]))))=, {&message_%qI %qD=%1; @pemit %#=The validator '%0' now has the message:%R%1;}, {@pemit %#=ERROR > %qE;}
@set VDBC/cmd-+validator/message=no_parse

&cmd-+validator/tag VDBC=$+validator/tag *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to tag., if(not(t(%1)), You must enter one or more tags to add to the validator. Tags are separated by spaces., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to tag., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(T, xget(%qD, tags_%qI))])))))=, {&tags_%qI %qD=setr(T, setunion(%qT, %1)); @pemit %#=The validator '%0' has been tagged with '%1'. Its tags are now:%R%T[iter(%qT, itext(0), , %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/untag VDBC=$+validator/untag *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to untag., if(not(t(%1)), You must enter one or more tags to remove from the validator. Tags are separated by spaces., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to untag., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(T, xget(%qD, tags_%qI))][if(strmatch(setdiff(%qT, %1), %qT), The tag or tags '%1' were not found in the validator '%0'.)])))))=, {&tags_%qI %qD=setr(T, setdiff(%qT, %1)); @pemit %#=The validator '%0' has lost the tag '%1'. Its tags are now:%R%T[iter(%qT, itext(0), , %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/stat VDBC=$+validator/stat *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to link to a stat., if(not(t(%1)), You must enter a stat to link to the validator., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to link to a stat., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(V, xget(%qD, stat_%qI))][if(not(t(setr(N, ulocal(fn-findstatbyname, %1)))), Could not find the stat you wish to link to the validator.)])))))=, {&stat_%qI %qD=setr(V, setunion(%qV, %qR)); @pemit %#=The validator '%0' has been linked with the stat '[ulocal(fn-getstatname, %qR)]'. It is now linked with the following stats:%R%T[iter(%qV, itext(0), , %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/unstat VDBC=$+validator/unstat *=*:@switch/first setr(E, if(not(isstaff(%#)), You must be staff to edit validators., if(not(t(%0)), You must enter a validator to unlink from a stat., if(not(t(%1)), You must enter a stat to unlink from the validator., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an already-existing validator to unlink from a stat., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][setq(V, xget(%qD, stat_%qI))][if(not(t(setr(N, ulocal(fn-findstatbyname, %1)))), Could not find the stat you wish to link to the validator., if(strmatch(setdiff(%qV, setr(R, rest(%qN, |))), %qV), The stat '[ulocal(fn-getstatname, %qR)]' was not found linked to the validator '%0'.)]))))))=, {&stat_%qI %qD=setr(V, setdiff(%qV, %qR)); @pemit %#=The validator '%0' has been unlinked from the stat '[ulocal(fn-getstatname, %qR)]'. It is now linked with the following stats:%R%T[iter(%qV, itext(0), , %R%T)];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/test VDBC=$+validator/test *=*:@switch/first setr(E, if(not(t(%0)), You must enter a validator to test., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an existing validator to test., setq(I, rest(%qS, |))[setq(D, first(%qS, |))][if(not(setr(P, pmatch(case(%1, me, %#, *%1)))), Could not find the player '%1'., if(and(not(isstaff(%#)), not(strmatch(%qP, %#))), You may not test a validator on anyone but yourself unless you are staff. Try +validator/test %qI.)[setq(N, name(%qP))])])))=, {@pemit %#=[setq(R, ulocal(%qD/validator_%qI, %qP))][if(t(%qR), The validator passed for %qN. The user would not see a message about this result. Actual output: %qR, The validator failed for %qN. Actual output: %qR%RThe user would see the following error message:%R%R[udefault(%qD/message_%qI, Validator failed: %qR, %qP, %qR)])];}, {@pemit %#=ERROR > %qE;}

&cmd-+validator/test VDBC=$+validator/test *:@switch/first %0=*=*, {}, {@switch/first setr(E, if(not(t(%0)), You must enter a validator to test., if(not(t(setr(S, ulocal(fn-findvalidatorbyid, %0)))), You must choose an existing validator to test., setq(I, rest(%qS, |))[setq(D, first(%qS, |))])))=, {@pemit %#=[setq(R, ulocal(%qD/validator_%qI, %#))][if(t(%qR), You passed validator %qI. You would not see a message about this result. Actual output: %qR, The validator failed for you. Actual output: %qR%RYou would see the following error message:%R%R[udefault(%qD/message_%qI, Validator failed: %qR, %#, %qR)])];}, {@pemit %#=ERROR > %qE;}}



@@ Execution method: @dolist [list of checks filtered by stat/tag]={run check. Break if critical error; otherwise show warning.}




@@ Default values for a simple nWoD character generation below.

+stat/add Full name
+stat/add Birthdate
+stat/add Virtue
+stat/add Vice
+stat/val Vice=Greed|Envy|Gluttony|Wrath|Lust
+stat/add Defense
+stat/formula Defense=min(getstat(%0, Dexterity), getstat(%0, Wits))
+stat/add Clan
+stat/val Clan=Daeva|Gangrel|Mekhet|Nosferatu|Ventrue


+stat/add Strength
+stat/add Presence
+stat/add Intelligence
+stat/add Dexterity
+stat/add Manipulation
+stat/add Wits
+stat/add Stamina
+stat/add Composure
+stat/add Resolve

+stat/tag Strength=Physical Attribute Power
+stat/tag Presence=Social Attribute Power
+stat/tag Intelligence=Mental Attribute Power
+stat/tag Dexterity=Physical Attribute Finesse
+stat/tag Manipulation=Social Attribute Finesse
+stat/tag Wits=Mental Attribute Finesse
+stat/tag Stamina=Physical Attribute Resistance
+stat/tag Composure=Social Attribute Resistance
+stat/tag Resolve=Mental Attribute Resistance

+stat/min Strength=1
+stat/min Presence=1
+stat/min Intelligence=1
+stat/min Dexterity=1
+stat/min Manipulation=1
+stat/min Wits=1
+stat/min Stamina=1
+stat/min Composure=1
+stat/min Resolve=1

+stat/max Strength=10
+stat/max Presence=10
+stat/max Intelligence=10
+stat/max Dexterity=10
+stat/max Manipulation=10
+stat/max Wits=10
+stat/max Stamina=10
+stat/max Composure=10
+stat/max Resolve=10

+validator/set 1=[setq(B, case(ulocal(fn-getstat, %0, Clan), Daeva, Dexterity|Manipulation, Gangrel, Composure|Stamina, Mekhet, Intelligence|Wits, Nosferatu, Composure|Strength, Ventrue, Presence|Resolve))][setq(S, ulocal(fn-getstat, %0, Strength))][setq(T, ulocal(fn-getstat, %0, Stamina))][setq(D, ulocal(fn-getstat, %0, Dexterity))][setq(P, ulocal(fn-getstat, %0, Presence))][setq(M, ulocal(fn-getstat, %0, Manipulation))][setq(C, ulocal(fn-getstat, %0, Composure))][setq(I, ulocal(fn-getstat, %0, Intelligence))][setq(W, ulocal(fn-getstat, %0, Wits))][setq(R, ulocal(fn-getstat, %0, Resolve))][setq(H, ladd(iter(%qS %qT %qD, case(itext(0), 5, 5, sub(itext(0), 1)))))][setq(O, ladd(iter(%qP %qM %qC, case(itext(0), 5, 5, sub(itext(0), 1)))))][setq(E, ladd(iter(%qI %qW %qM, case(itext(0), 5, 5, sub(itext(0), 1)))))][case(%qB, , case(sort(%qH %qO %qE, n), 3 4 5, 1, #-1 You must spend all your attribute points in a 5/4/3 spread. Your spread was: [revwords(sort(%qH %qO %qE, n), , /)]), if(not(t(trim(iter(%qB, if(case(itext(0), Strength, 1[if(eq(%qS), 5, setq(H, sub(%qH, 1)))], Stamina, 1[if(eq(%qT), 5, setq(H, sub(%qH, 1)))], Dexterity, 1[if(eq(%qD), 5, setq(H, sub(%qH, 1)))]), case(%qH %qO %qE, 6 4 3, 1, 6 3 4, 1, 5 5 3, 1, 5 3 5, 1, 4 5 4, 1, 4 4 5, 1), if(case(itext(0), Presence, 1[if(eq(%qP), 5, setq(O, sub(%qO, 1)))], Manipulation, 1[if(eq(%qM), 5, setq(O, sub(%qO, 1)))], Composure, 1[if(eq(%qC), 5, setq(O, sub(%qO, 1)))]), case(%qO %qH %qE, 6 4 3, 1, 6 3 4, 1, 5 5 3, 1, 5 3 5, 1, 4 5 4, 1, 4 4 5, 1), if(case(itext(0), Intelligence, 1[if(eq(%qI), 5, setq(E, sub(%qE, 1)))], Wits, 1[if(eq(%qW), 5, setq(E, sub(%qE, 1)))], Resolve, 1[if(eq(%qR), 5, setq(E, sub(%qE, 1)))]), case(%qE %qO %qH, 6 4 3, 1, 6 3 4, 1, 5 5 3, 1, 5 3 5, 1, 4 5 4, 1, 4 4 5, 1, 0)))), |, @@), b, 0))), #-1 You must spend all your attribute points in a 5/4/3 spread plus one point in [itemize(%qB, |, or)]. Your spread was %qH Physical/%qO Social/%qE Mental%, which equates to: [revwords(sort(%qH %qO %qE, n), , /)], 1))]
+validator/test 1

+validator/desc 1=Check the balance of attribute points.

+validator/add setq(0, setq(1, setq(2, setq(3, ))))[null(iter(ulocal(fn-liststatsbytag, Attribute), setq(3, xget(first(itext(0), |), stat_[rest(itext(0), |)]))[setq(1, default(%0/_stat_[rest(itext(0), |)], 0))][if(gt(%q1, 5), setq(0, setunion(%q0, %q3, |)), if(lt(%q1, 1), setq(2, setunion(%q2, %q3, |))))]))][if(or(t(%q0), t(%q2)), #-1 [if(t(%q0), You may not have attributes above 5 in chargen: [itemize(%q0, |)].)] [if(t(%q2), You may not have attributes below 1 in chargen: [itemize(%q2, |)].)], 1)]
+validator/desc 2=Don't allow attributes above 5 or below 1 in chargen.

+stat/add Academics
+stat/add Athletics
+stat/add Animal Ken
+stat/add Computer
+stat/add Brawl
+stat/add Empathy
+stat/add Crafts
+stat/add Drive
+stat/add Expression
+stat/add Investigation
+stat/add Firearms
+stat/add Intimidation
+stat/add Medicine
+stat/add Larceny
+stat/add Persuasion
+stat/add Occult
+stat/add Stealth
+stat/add Socialize
+stat/add Politics
+stat/add Survival
+stat/add Streetwise
+stat/add Science
+stat/add Weaponry
+stat/add Subterfuge

+stat/tag Academics=Skill Mental
+stat/tag Athletics=Skill Physical
+stat/tag Animal Ken=Skill Social
+stat/tag Computer=Skill Mental
+stat/tag Brawl=Skill Physical
+stat/tag Empathy=Skill Social
+stat/tag Crafts=Skill Mental
+stat/tag Drive=Skill Physical
+stat/tag Expression=Skill Social
+stat/tag Investigation=Skill Mental
+stat/tag Firearms=Skill Physical
+stat/tag Intimidation=Skill Social
+stat/tag Medicine=Skill Mental
+stat/tag Larceny=Skill Physical
+stat/tag Persuasion=Skill Social
+stat/tag Occult=Skill Mental
+stat/tag Stealth=Skill Physical
+stat/tag Socialize=Skill Social
+stat/tag Politics=Skill Mental
+stat/tag Survival=Skill Physical
+stat/tag Streetwise=Skill Social
+stat/tag Science=Skill Mental
+stat/tag Weaponry=Skill Physical
+stat/tag Subterfuge=Skill Social

+stat/min Academics=0
+stat/min Athletics=0
+stat/min Animal Ken=0
+stat/min Computer=0
+stat/min Brawl=0
+stat/min Empathy=0
+stat/min Crafts=0
+stat/min Drive=0
+stat/min Expression=0
+stat/min Investigation=0
+stat/min Firearms=0
+stat/min Intimidation=0
+stat/min Medicine=0
+stat/min Larceny=0
+stat/min Persuasion=0
+stat/min Occult=0
+stat/min Stealth=0
+stat/min Socialize=0
+stat/min Politics=0
+stat/min Survival=0
+stat/min Streetwise=0
+stat/min Science=0
+stat/min Weaponry=0
+stat/min Subterfuge=0

+stat/max Academics=10
+stat/max Athletics=10
+stat/max Animal Ken=10
+stat/max Computer=10
+stat/max Brawl=10
+stat/max Empathy=10
+stat/max Crafts=10
+stat/max Drive=10
+stat/max Expression=10
+stat/max Investigation=10
+stat/max Firearms=10
+stat/max Intimidation=10
+stat/max Medicine=10
+stat/max Larceny=10
+stat/max Persuasion=10
+stat/max Occult=10
+stat/max Stealth=10
+stat/max Socialize=10
+stat/max Politics=10
+stat/max Survival=10
+stat/max Streetwise=10
+stat/max Science=10
+stat/max Weaponry=10
+stat/max Subterfuge=10

