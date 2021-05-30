@@ REQUIRED FEATURES - READ THIS FIRST!
@@ -
@@ You are going to need a Function Invocation Level of at least 10,000.
@@ -
@@ This code assumes you've created a stdheader, stdsubheader, and stdfooter for your game.
@@ -
@@ You are assumed to have an isstaff(dbref) function. The SGP Globals come with one.
@@ -
@@ GAME-SPECIFIC DIFFERENCES
@@ -
@@ This section is likely only interesting to coders, and maybe not even those. I apologize in advance for any system bias I am perceived to be displaying. Read on at your own risk.
@@ -
@@ itemize(<list>, <delim>) - equates to elist(<list>, <phrase>, <delim>) on Rhost. Rhost users may need to create a custom itemize function. There should be one included with the SGP Globals, however.
@@ iter() calls which are required to have no spaces have an output delimiter of @@ for MUX compatibility. Rhost users will need to change this to @@().
@@ -
@@ Since there's no ibreak() on MUX, I've used first(iter(blah, if(bleh, bloo|<term>|)), |<term>|). Waste of cycles, but easy to convert - just search for the first |<term>| and stick a [ibreak()] in its place, then remove the 'first(' and the ', |<term>|)'. Note that you will have to do this by hand (or at least look before hitting replace all) because first( is used elsewhere in the code. Or you could just leave it as is - not like nowadays we can't afford the extra cycles.
@@ -
@@ streq() has been replaced with eq(comp(string1, string2), 0) for MUX compatibility.
@@ -
@@ Reverse strip (strip only characters NOT in the permitted list) has been replaced by eq(comp(strip(input, list of valid characters),), 0)
@@ -
@@ SPEC NOTES
@@ -
@@ There are two types of stat in NWoD: simple and compound. A simple stat is a stat which has but one value, whether it's text or numerical. A compound stat is a simple stat - with the ability to attach additional stats to it.
@@ -
@@ Simple stats are expressed like so: Stat Type:Stat Name:Value
@@ -
@@ Compound stats are expressed like so: Stat Type:Stat Name:Value:List of simple stats separated by tildes
@@ -
@@ To properly explain these, we're going to have to dissect the various parts of a stat.
@@ -
@@ Stat Type: A stat type is the name of the group of stats (Attribute, Skill, Merit, etc) your chosen stat belongs to. We want to reuse these as much as possible, since they make it easy to group stats and auto-generate a sheet based on what the player's got stored on them. There are certain reserved stat types, all of which are used to help check a player for certain settings which will allow them to get restricted stats: Flag, which may be used to check a player for specific flags; Setting, which will look for an attribute on the player object; and I'm leaving room to add more, though I can't think of any right now.
@@ -
@@ Stat Name: Strength, Brawl, etc. However! Some stats have special names. These include names like "Language (*)" and "Status (Police|City|Harpies|whatever)". We're going to work up a parsing code to allow you to make stat names change depending on value. Note that this only matters during chargen (and advancement) - on the player's sheet (or sheet object), they're going to have Merit:Language (German):1. Thus there will be no direct analog for that stat in the stat database - but that's okay, the parser will be able to spot it.
@@ -
@@ Stat Value: Value has three options:
@@ -
@@ 1. A number (#) which is between 1 and 10, and may be treated as a pool, with temporary raises and losses. NO NWoD stat may go above 10. Most won't ever exceed 5. However, that's for the code you build off of this character generator to worry about - in the stat DB, numerical stats will be stored as "#". That implies that 0/10 is a correct value, as is 6, as is 0, as is 3. If you write code to raise or lower someone's stats according to game rules, make sure you account for the game rules within that code - the chargen will NOT prevent them from raising or rolling a stat at 10.
@@ -
@@ 2. A word or phrase (*) which may have any of the following characters: A-Z, 0-9, !, @, $, ^, `, ', ", ,, ., ?, -, +, space, and _. There are some characters missing. The reason for this? We are going to use them as separators, OR they are capable of causing the chargen's sheet to barf. The separator characters are :, |, and ~. The characters used to parse stat names and values are *, #, (, ), <, >, =, &, and |. The characters which may cause the chargen to barf are %, \, {, }, [, ], /, and ;. Everything else is whitelisted. If you want to add to that list of permissible characters, there's a setting for it - but think very carefully before you do and make sure that the character in question won't cause your game harm. (European characters should be all right - I just can't be arsed to add them myself and they might not work on some games, so I'm leaving them out for now.)
@@ -
@@ 3. A list of permissible text values, such as "Daeva|Gangrel|Mekhet|Nosferatu|Ventrue". Sorry, no parsing of pipes allowed on this one. I couldn't see any reason for the feature and so left it out.
@@ -
@@ And finally, that list of simple stats separated by tildes (~): these are formatted EXACTLY the way regular simple stats are. They use the same parser and everything. They're separated by tilde and kept in the fourth slot ALWAYS so that you can use the same functions to parse them.
@@ -
@@ Each stat is stored on its own attribute. Attribute name doesn't matter except that it begins with _stat-. Do NOT let your players add attributes that start with _stat-! They will be able to roll and prove them. Fortunately, attributes starting with _ are wizard-only on most games. That is assumed to be true here. If this assumption is incorrect, better edit your .conf file to protect those attributes, or there will be problems when players find that they can set their abilities to insane levels without spending XP.
@@ -
@@ We WILL have a standardized validation format to compare stats. This is going to be the trickiest part of all. We will need it for merits, and to determine whether a person has the correct race/group to have stat X. Right now, it is as follows:
@@ -
@@ CompareValue ComparisonOperator CompareValue
@@ -
@@ A CompareValue may be expressed in the following ways:
@@ -
@@ Stat Type:Stat Name - yes, you must use the fully qualified stat name for the stat you want. If you want it to apply to all Languages, you would use Merit:Language (*) - whereas if you want it to apply only to Language (German) you would need to express it as Merit:Language (German). It gets complicated, I know, having to repeat the entire thing, so I'll hopefully be including some shortcuts when I code the stat-adding portion of the CG.
@@ -
@@ A number, any number.
@@ -
@@ A piece of text, any valid piece of text using the rules above.
@@ -
@@ Flag:FLAG - one word, no spaces. If you want to set special permissions on your players, this is one way to do it. For example, Flag:Blind would require that the player object has the BLIND flag in order to operate it. Not the greatest example, but presumably you have special your-game-only flags if you want to use this.
@@ -
@@ Setting:SETTING - one word, no spaces. It's strongly recommended that you make the expected setting a wiz-only setting by starting it with a _. For example, you could check that the player has a setting of _TEMPLATE with the value "Feature Character" if you wanted. Staff would, of course, have to set the _TEMPLATE attribute on the player themselves, but you can easily make certain merits or stats check for it.
@@ -
@@ -
@@ A ComparisonOperator is one of the following: >, <, =
@@ -
@@ You can group these expressions and then join them as in the standard programming language expressions. To group, use parentheses. Joining operators are | (OR) and & (AND). So you might end up with expressions like:
@@ -
@@ Merit:Stunning Looks > 2 & Skill:Persuasion > 3
@@ -
@@ (Attribute:Strength > 2 & Skill:Brawl > 2) | Attribute:Dexterity > 3
@@ -
@@ Personal:Group = Police
@@ -
@@ Setting:_TEMPLATE = Feature Character
@@ -
@@ COMMAND SPEC
@@ -
@@ Goal: Keep the list of commands simple and easy to remember. Multiple, easy to remember commands are preferred over single, overly complex commands.
@@ Goal: Don't step on any default commands like +set. Let's try to keep them prefixed with +cg and +xp.
@@ Goal: Follow the model set forth in GCG v1 - advanced players can set things to whatever they want, without being pestered with errors. Staff can, if they choose, manually approve these characters.
@@ Goal: Have a small suite of staff-only commands to override default chargen behavior.
@@ -
@@ NEW PLAYER COMMANDS
@@ -
@@ +cghelp - Quick reference of chargen commands.
@@ +cgset Stat Name=Value
@@   +cgset Brawl=5
@@   +cgset Language (German)=1
@@   +cgset Hollow (Personal)=5
@@ +cgset Stat Name/Detail=Value
@@   +cgset Hollow/Wards=2
@@   +cgset Brawl/Specialty=Right Hook
@@   +cgset Humanity/Megalomania=5
@@ +cgcheck
@@ +cgcheck Stat Type
@@ +cgcheck Stat Name
@@ +cgcheck Stat Name/Detail
@@ +cgcomplete - Runs all checks and, if you pass, performs the "chargen complete" action.
@@ +cgwipe - Wipe your stats. Will ask if you're sure first.
@@ +cgtemplate - Lists all templates.
@@ +cgtemplate <Template> - Show a template.
@@ -
@@ -
@@ -
@@ ALL PLAYER COMMANDS
@@ -
@@ +ref - List all types of stats, plus handy +statdb help info.
@@ +ref Stat Type - List all stats in that type, plus any checks that will be run by type.
@@ +ref Stat Name - List all details in that stat, plus formats available, plus values available, plus any checks that will be run on this stat.
@@ +ref Stat Name/Detail - List all details on this detail, plus formats available, plus values available, plus any checks that will be run on this stat.
@@ -
@@ -
@@ STAFF COMMANDS
@@ -
@@ +cgstaffhelp - Quick reference of staff chargen commands.
@@ +cgapprove Player
@@ +cgunapprove Player
@@ +newstat Stat Type:Stat Name=Value
@@ +newstat Stat Type:Stat Name/Stat Type:Detail=Value
@@ +newtype Stat Type
@@ +cgrestrict Stat Name=Restrictions (adding new restrictions compounds the previous restrictions in an additive fashion)
@@ +cgunrestrict Stat Name
@@ +cgnote Stat Name=Notes (enter nothing to remove)
@@ +cgnuke Stat Name
@@ +cgnuke Stat Name/Detail
@@ +cgnuketype Stat Type
@@ -
@@ -
@@ -
@@ -
@@ -

@create Stat DataBase <STDB>=10
@set STDB=HALT

@force me=&vS STDB=num(STDB)

&strict_characters STDB=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 !@$^`'",.?-+_

&regex_characters STDB=[v(strict_characters)]|*#()

&compare_characters STDB=[v(strict_characters)]|*#()<>=&

@create StatDB Functions <SDBF>=10
@set SDBF=HALT INHERIT
@parent SDBF=STDB

&fn-stdheader SDBF=%b.[center(< %0 >, 75, -)].

&fn-stdfooter SDBF=%b.[if(t(%0), center(< %0 >, 75, -), repeat(-, 75))].

&fn-threecolumns SDBF=iter(%0, [ljust(mid(itext(0), 0, setr(0, if(eq(mod(inum(0), 3), 2), 24, 23))), %q0)][if(eq(mod(inum(0), 3), 0), %R %b, %b%b)], |, @@())

&fn-valid_characters SDBF=eq(comp(strip(%0, u(%1_characters)),), 0)

&fn-is_type SDBF=t(match(v(stat_types), %0, |))

&fn-is_stat SDBF=t(match(iter(lattr(%vS/stat-*), extract(v(itext(0)), 2, 1, :),, :), %0, :))

&fn-find_stat SDBF=if(t(setr(0, match(iter(lattr(%vS/stat-*), extract(v(itext(0)), 2, 1, :),, :), %0, :))), extract(lattr(%vS/stat-*), %q0, 1))

&fn-is_stat_detail SDBF=t(match(extract(v(%0), 4, 9999, :), *:%1:*, ~))

&fn-get_stat_name SDBF=extract(v(%0), 2, 1, :)




@create StatDB Commands <SDBC>=10
@set SDBC=INHERIT
@parent SDBC=SDBF

&cmd-+ref SDBC=$+ref:@pemit %#=u(fn-stdheader, Stat Reference)%R Available stat types:%R%R%b [ulocal(fn-threecolumns, v(stat_types))]%R%R Type +ref Stat Type or Stat Name for more.%R[u(fn-stdfooter)];

&cmd-+newtype SDBC=$+newtype *:@switch setr(E, if(not(u(fn-valid_characters, %0, strict)), You must include valid characters for the stat type. Your invalid characters are as follows: [strip(%0, u(strict_characters))]%b, if(u(fn-is_type, %0), %0 is already a stat type.%b)))=,{&stat_types %vS=[if(t(v(stat_types)), v(stat_types)|%0, %0)]; @pemit %#=Added your stat type "%0".;},{@pemit %#=PARSE ERROR > %qE;};

+newtype Attribute
+newtype Skill
+newtype Merit
+newtype Personal
+newtype Contract
+newtype Pool
+newtype Derangement
+newtype Detail
+newtype Specialty



&cmd-+newstat SDBC=$+newstat *=*:@switch setq(S, if(strmatch(%0, */*), first(%0, /)[setq(D, rest(%0, /))], %0))[setr(E, if(and(not(t(%qD)), not(strmatch(%qS, *:*))), You must include a stat type with your new stat.%b, setq(T, first(%qS, :))[setq(S, rest(%qS, :))])[if(not(u(fn-valid_characters, %qS, regex)), You must include valid characters for the stat name. Your invalid characters are as follows: [strip(%qS, u(regex_characters))]%b)][if(t(%qD), if(not(strmatch(%qD, *:*)), You must include a stat type with your new stat detail.%b, setq(Y, first(%qD, :))[setq(D, rest(%qD, :))])[if(not(u(fn-valid_characters, %qD, regex)), You must include valid characters for the stat detail's name. Your invalid characters are as follows: [strip(%qD, u(regex_characters))]%b)])][if(not(u(fn-valid_characters, %1, regex)), You must include valid characters for the stat value. Your invalid characters are as follows: [strip(%1, u(regex_characters))]%b)])][if(strmatch(%qE,), setr(E, if(and(not(t(%qD)), not(u(fn-is_type, %qT))), %qT is not a valid stat type.%b)[if(t(%qD), if(not(u(fn-is_type, %qY)), %qY is not a valid stat type.%b))]))][if(strmatch(%qE,), setr(E, if(and(not(t(%qD)), u(fn-is_stat, %qS)), %qS is already a stat.%b, if(t(%qD), if(not(setr(F, u(fn-find_stat, %qS))), %qS is not a valid stat.%b, if(u(fn-is_stat_detail, %qF[setq(S, u(fn-get_stat_name, %qF))], %qD), %qD is already a detail for %qS.%b))))))]=,{@switch %qD=,{&stat-[add(default(%vS/stat_count, 0), 1)] %vS=%qT:%qS:%1; @pemit %#=Added the simple stat "%qS" with the stat type of "%qT" and a possible value of "%1".;},{&%qF %vS=[if(gt(words(v(%qF), :), 4), v(%qF)~%qY:%qD:%1, v(%qF):%qY:%qD:%1)]; @pemit %#=Added the simple stat "%qD" with the stat type of "%qY" and a possible value of "%1" to the complex stat "%qS".;};},{@pemit %#=PARSE ERROR > %qE;};


+newstat Attribute:Strength=#
+newstat Attribute:Dexterity=#
+newstat Attribute:Stamina=#
+newstat Attribute:Presence=#
+newstat Attribute:Manipulation=#
+newstat Attribute:Composure=#
+newstat Attribute:Intelligence=#
+newstat Attribute:Wits=#
+newstat Attribute:Resolve=#

+newstat Skill:Brawl=#
@@ BUG NEXT LINE
+newstat Brawl/Specialty:*=1



@create Chargen DB Holder <CDH>=10
@set CDH=HALT INHERIT
@force me=&CDH me=num(CDH)

&stafflock CDH=isstaff(%0)
@lock/use CDH=stafflock/1

&fn-getstatdbref CDH=if(t(setr(0, first(iter(lcon(me), if(strmatch(name(itext(0)), %0), itext(0)|<term>|)), |<term>|))), %q0, #-1 COULD NOT GET STAT DBREF FOR %0.)

&fn-getstat CDH=if(t(%2), if(t(setr(0, ulocal(fn-getstatdbref, %2))), ulocal(%q0/fn-getstat, %0, %1), %q0), if(t(setr(0, first(iter(lcon(me), if(t(member(get(itext(0)/valid_stat_names), %1, |)), ulocal(itext(0)/fn-getstat, %0, %1)|<term>|)), |<term>|))), %q0, if(t(words(%q0)), %q0, #-1 COULD NOT FIND STAT %1 for %0. STAT TYPE NOT PROVIDED.)))

&fn-getstatdetail CDH=if(t(%2), if(t(setr(0, ulocal(fn-getstatdbref, %2))), ulocal(%q0/fn-getstatdetail, %0, %1), %q0), if(t(setr(0, first(iter(lcon(me), if(t(member(lattr(itext(0), detail-*), detail-%1)), ulocal(itext(0)/fn-getstatdetail, %0, %1)|<term>|)), |<term>|))), %q0, if(t(words(%q0)), %q0, #-1 COULD NOT FIND STAT DETAIL %1 for %0. STAT TYPE NOT PROVIDED.)))




@create NWoD Races <NR>=10
@set NR=HALT INHERIT
@force me=@parent NR=v(CDH)
@lock/use NR=stafflock/1
@force me=&NR me=num(NR)

&valid_stat_names NR=Race

&valid_stat_values NR=Mortal|Changeling

&detail-attribute_balance NR=5 4 3

&detail-skill_balance NR=11 7 4

&detail-num_specialties NR=3

&fn-getstatdbref CDH=if(t(setr(0, first(iter(lcon(me), if(strmatch(name(itext(0)), %0), itext(0)|<term>|)), |<term>|))), %q0, #-1 COULD NOT GET RACE OBJECT DBREF FOR %0.)

&fn-getstat NR=default(%0/_race, Mortal)

&fn-getstatdetail NR=if(t(setr(0, ulocal(fn-getstatdbref, %0))), udefault(%q0/detail-%1, udefault(me/detail-%1, #-1 ERROR GETTING RACE-LEVEL DETAIL %1 FOR %0.)), %q0)




@create NWoD Mortal <NM>=10
@set NM=HALT INHERIT
@force me=@parent NM=v(NR)
@lock/use NM=stafflock/1




@create NWoD Changeling <NC>=10
@set NC=HALT INHERIT
@force me=@parent NC=v(NR)
@lock/use NC=stafflock/1

&detail-attribute_balance NC=5 4 3




@create NWoD Attributes <NA>=10
@set NA=HALT INHERIT
@force me=@parent NA=v(CDB)
@lock/use NA=stafflock/1

&valid_stat_names NA=Intelligence|Wits|Resolve|Strength|Dexterity|Stamina|Presence|Manipulation|Composure

&valid_stat_values NA=1|2|3|4|5|6|7|8|9|10

&fn-getstat NA=extract(setr(0, default(%0/_attributes, 1|1|1|1|1|1|1|1|1)), member(%q0, %1, |), 1, |)

&fn-get_valid_values NA=default(me/valid_%0_values, v(valid_stat_values))

&fn-get_cg_cost NA=if(t(setr(0, ulocal(fn-getstat, %0, %1, attribute))), if(eq(%q0, 5), 5, sub(%q0, 1)), 0)

&cgcheck-valid_values NA=iter(v(valid_stat_names), if(!t(member(setr(1, ulocal(fn-get_valid_values)), setr(0, ulocal(fn-getstat, %0, itext(0))), |)), You must enter one of [itemize(%q1, |)] for [itext(0)].%R, if(lte(%q0, 0),The minimum allowable value for [itext(0)] is 1.%R, if(gt(%q0, 5), The maximum allowable value for [itext(0)] at character generation is 5.%R))), |, @@)

&cgcheck-balance NA=if(!eq(comp(setr(0, ulocal(fn-getstatdetail, %0, attribute_balance)), setr(1, sort(ladd(ulocal(fn-get_cg_cost, %0, Strength) [ulocal(fn-get_cg_cost, %0, Dexterity)] [ulocal(fn-get_cg_cost, %0, Stamina)]) [ladd(ulocal(fn-get_cg_cost, %0, Intelligence) [ulocal(fn-get_cg_cost, %0, Wits)] [ulocal(fn-get_cg_cost, %0, Resolve)])] [ladd(ulocal(fn-get_cg_cost, %0, Presence) [ulocal(fn-get_cg_cost, %0, Manipulation)] [ulocal(fn-get_cg_cost, %0, Composure)])], n))), 0), You must spread your points among attributes in three groups of [itemize(%q0)]. You have [itemize(%q1)].%R)

&layout NA=stdsubheader(Attributes)%R %b[space(12)][center(Mental,20)] %b[center(Physical,19)] %b[center(Social,20)]%R %b[rjust(Power:,11)] [ljust(Intelligence:,14)] [rjust(ulocal(fn-getstat,%0,Intelligence),5)] %b[ljust(Strength:,13)] [rjust(ulocal(fn-getstat,%0,Strength),5)] %b[ljust(Presence:,14)] [rjust(ulocal(fn-getstat,%0,Presence),5)]%R %b[rjust(Finesse:,11)] [ljust(Wits:,14)] [rjust(ulocal(fn-getstat,%0,Wits),5)] %b[ljust(Dexterity:,13)] [rjust(ulocal(fn-getstat,%0,Dexterity),5)] %b[ljust(Manipulation:,14)] [rjust(ulocal(fn-getstat,%0,Manipulation),5)]%R %bResistance: [ljust(Resolve:,14)] [rjust(ulocal(fn-getstat,%0,Resolve),5)] %b[ljust(Stamina:,13)] [rjust(ulocal(fn-getstat,%0,Stamina),5)] %b[ljust(Composure:,14)] [rjust(ulocal(fn-getstat,%0,Composure),5)]

@create NWoD Skills <NS>=10
@set NS=HALT INHERIT
@force me=@parent NS=v(CDB)
@lock/use NS=stafflock/1
@force me=&NS me=num(NS)

&valid_stat_names NS=Academics|Athletics|Animal Ken|Computer|Brawl|Empathy|Crafts|Drive|Expression|Investigation|Firearms|Intimidation|Medicine|Larceny|Persuasion|Occult|Stealth|Socialize|Politics|Survival|Streetwise|Science|Weaponry|Subterfuge

&valid_stat_values NS=0|1|2|3|4|5

&fn-getstat NS=extract(setr(0, default(%0/_skills, 0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0)), member(%q0, %1, |), 1, |)

&fn-get_valid_values NS=default(me/valid_%0_values, v(valid_stat_values))

&cgcheck-valid_values NS=iter(v(valid_stat_names), if(!t(member(setr(1, ulocal(fn-get_valid_values)), setr(0, ulocal(fn-getstat, %0, itext(0))), |)), You must enter one of [itemize(%q1, |)] for [itext(0)].%R, if(lte(%q0, 0),The minimum allowable value for [itext(0)] is 1.%R, if(gt(%q0, 5), The maximum allowable value for [itext(0)] at character generation is 5.%R))), |, @@)

&cgcheck-balance NS=[setq(0, ladd(iter(Academics Computer Crafts Investigation Medicine Occult Politics Science, setr(3, ulocal(fn-getstat, %0, itext(0))) [gte(%q3, 5)])))][setq(1, ladd(iter(Athletics Brawl Drive Firearms Larceny Stealth Survival Weaponry, setr(3, ulocal(fn-getstat, %0, itext(0))) [gte(%q3, 5)])))][setq(2, ladd(iter(Animal Ken|Empathy|Expression|Intimidation|Persuasion|Socialize|Streetwise|Subterfuge, setr(3, ulocal(fn-getstat, %0, itext(0))) [gte(%q3, 5)], |)))][if(case(setr(1, revwords(sort(%q0 %q1 %q2, n))), setr(0, ulocal(fn-getstatdetail, %0, skill_balance)), 0, 1), You must spread your points among skills in three groups of [itemize(%q0)]. You have [itemize(%q1)].%R)]

&layout NS=stdsubheader(Skills)%R %b[ljc(center(Mental %(-3 unskilled%),24),24)] %b[ljc(center(Physical %(-1 unskilled%),23),23)] %b[ljust(center(Social %(-1 unskilled%),24),24)][iter(v(valid_stat_names), if(eq(mod(inum(0), 3), 1), %R %b)[ljust(itext(0)[if(hasattr(%0, _specialty-[edit(itext(0), %b, _)]), *)]:, if(!eq(mod(inum(0), 3), 2), 19, 18))][rjust(ulocal(fn-getstat, %0, itext(0)), 5)]%b%b, |, @@)]


@create NWoD Skill Specialties <NSS>=10
@set NSS=HALT INHERIT
@force me=@parent NSS=v(CDB)
@lock/use NSS=stafflock/1
@force me=&vS NSS=v(NS)

&valid_stat_names NSS=iter(get(%vS/valid_stat_names), itext(0) Specialty, |, |)

&valid_stat_characters NSS=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .;:'"?/\-_+!@#$^&*()<>

&valid_stat_length NSS=255

&fn-getstat NSS=get(%0/_specialty-[edit(%1, %b, _)])

&fn-get_specialty NSS=extract(setr(0, ulocal(fn-getstat, %0, %1)), match(%q0, %2), 1, |)

&fn-get_skill_value NSS=ulocal(%vS/fn-getstat, %0, %1)

&fn-get_skill_name NSS=extract(setr(0, get(%vS/valid_stat_names)), match(%q0, %0), 1, |)

&cgcheck-points NSS=if(case(setr(0, ulocal(fn-getstatdetail, %0, num_specialties)), setr(1, words(lattr(%0/_specialty-*))), 0, 1), You must have %q0 specialties. You have %q1.%R)

&cgcheck-skills_and_duplicates NSS=iter(lattr(%0/_specialty-*), if(t(ulocal(fn-get_skill_value, %0, setr(0, edit(after(itext(0), -), _, %b)))), You must have at least a 1 in [setr(2, ulocal(fn-get_skill_name, %q0))] in order to have a specialty in it.%R)[iter(setr(1, get(%0/[itext(0)])), setq(0, 0)[iter(%q1, if(eq(comp(itext(0), itext(1)), 0), setq(0, add(%q0, 1))), |, @@)][if(gt(%q0, 1), You may not have multiple specialties with the same text ([itext(0)]) in %q2.%R)], |, @@)],, @@)

&layout NSS=stdsubheader(Skill Specialties)[iter(lattr(%0/_specialty-*), %R%b [setr(0, ulocal(fn-get_skill_name, edit(after(itext(0), -), _, %b)))]: [wrap(itemize(ulocal(fn-getstat, %0, %q0), |), sub(75, add(strlen(%q0), 2)))], |, @@)]



