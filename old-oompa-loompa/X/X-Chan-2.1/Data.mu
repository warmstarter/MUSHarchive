# Data constants for the system
#
&list_chanflags X-Channels Data Storage=
NO_ALIASES LIMITED_BROADCAST PRIVATE NO_EMITS PARANOID
-
&chanflag_no_aliases X-Channels Data Storage=
1
-
&comment_no_aliases X-Channels Data Storage=
Prevents the use of aliases on the channel.
-
&chanflag_limited_broadcast X-Channels Data Storage=
2
-
&comment_limited_broadcast X-Channels Data Storage=
Only staff or approved players may broadcast%ron the channel.
-
&chanflag_private X-Channels Data Storage=
4
-
&comment_private X-Channels Data Storage=
Staff cannot override channel lock.
-
&chanflag_no_emits X-Channels Data Storage=
8
-
&comment_no_emits X-Channels Data Storage=
No emits (~-poses) allowed.
-
&chanflag_paranoid X-Channels Data Storage=
16
-
&comment_paranoid X-Channels Data Storage=
Forces (Real Name) after alias when broadcasting.
-
# Functions we'll need
#
&fnc_chanflag X-Channels Data Storage=[
mod(
  div(
    v(chan_%0_flags),
    v(chanflag_%1)
  ),
  2
)
]
-
&fnc_abbrevflag X-Channels Data Storage=[
switch(%0,
  NO_ALIASES,
  A,
  LIMITED_BROADCAST,
  L,
  PRIVATE,
  X,
  NO_EMITS,
  E,
  PARANOID,
  P
)
]
-
&fnc_showflags X-Channels Data Storage=[
edit(
  iter(
    v(list_chanflags),
    switch(
      u(fnc_chanflag,%0,##),
      0,-,
      u(fnc_abbrevflag,##)
    )
  ),
  %b,
)
]
-
&fnc_checkstaff X-Channels Data Storage=[
or(
  hasflag(%0,judge),
  hasflag(%0,overworked),
  hasflag(%0,wizard)
)
]
-
&fnc_canseedark X-Channels Data Storage=[
or(
  hasflag(%0,overworked),
  hasflag(%0,wizard)
)
]
-
&fnc_filconn X-Channels Data Storage=[
hasflag(%0,connected)
]
-
&fnc_isadministrator X-Channels Data Storage=[
hasflag(%#,wizard)
]
-
&fnc_isnotdark X-Channels Data Storage=[
not(hasflag(%0,DARK))
]
-
&fnc_who_nodark X-Channels Data Storage=[
iter(filter(fnc_isnotdark,filter(fnc_filconn,v(chan_%0_list))),
  [switch(
    [setq(9,u(fnc_channame,##,%0))]
    [comp(name(##),%q9)],
    0,
    %q9,
    %q9 ([name(##)])
  )]
)
]
-
&fnc_who_dark X-Channels Data Storage=[
iter(filter(fnc_filconn,v(chan_%0_list)),
  [switch(
    [setq(9,u(fnc_channame,##,%0))]
    [comp(name(##),%q9)],
    0,
    %q9,
    %q9 ([name(##)])
  )]
  [switch(
  hasflag(##,DARK),
  1,
  %b(DARK),
  )]
)
]
-
&fnc_announce X-Channels Data Storage=[
or(
  not(hasflag(%#,DARK)),
  u(fnc_canseedark,%0)
)
]
-
&fnc_chanok X-Channels Data Storage=[
or(
  and(
    u(fnc_checkstaff,%1),
    not(u(fnc_chanflag,%0,PRIVATE))
  ),
  u(chan_%0_lock,%1),
  eq(strlen(v(chan_%0_lock)),0)
)
]
-
&fnc_broadcastok X-Channels Data Storage=[
or(
  u(fnc_checkstaff,%1),
  u(chan_%0_broadcastlock,%1)
)
]
-
&fnc_map_visible X-Channels Data Storage=[
u(fnc_chanok,%0,%#)
]
-
&fnc_channame X-Channels Data Storage=[
switch(
  [setq(9,get(%0/chanalias_%1))]
  0,
  [min(
    strlen(%q9),
    not(u(fnc_chanflag,%1,NO_ALIASES))
  )],
  name(%0),
  [not(u(fnc_chanflag,%1,PARANOID))],
  %q9%b([name(%0)]),
  %q9
)
]
-
&fnc_match_channel X-Channels Data Storage=[
switch(
  [match(lattr(%1/channel_*),channel_%0)][setq(8,v(list_channels))],
  0,
  switch(
    [setq(9,match(%q8,%0*))]%q9,
    0,
    INV_CHANNEL,
    [switch(
      match(ldelete(%q8,%q9),%0*),
      0,
      extract(%q8,%q9,1),
      MUL_CHANNEL
    )]
  ),
  extract(%q8,match(%q8,get(%1/channel_%0)),1)
)
]
-
&text_help X-Channels Data Storage=
[repeat(=,75)]%r
[center(X-Channels 2.0 Help,75)]%r
[repeat(=,75)]%r
+com/join <channel>%t%t-%tJoin <channel>%r
+com +<channel>%t%t%t-%tJoin <channel>%r
+com/leave <channel>%t%t-%tLeave <channel>%r
+com -<channel>%t%t%t-%tLeave <channel>%r
%r
=<channel> <message>%t%t-%tCommunicate on <channel>%r
%r
+com/list%t%t%t-%tList all channels you may join%r
+com/who <channel>%t%t-%tShow who is on <channel>%r
+com/flags%t%t%t-%tLists channel flags and their%r
%t%t%t%t%tmeanings%r
%r
+com/aliases%t%t%t-%tList all personal channel aliases%r
+com/addalias <alias> <channel>%t-%tSet <alias> to refer to <channel>%r
+com/rmalias <alias>%t%t-%tRemove <alias>%r
%r
+com/setname <channel> <name>%t-%tSet name on <channel> to <name>%r
+com/rmname <channel>%t%t-%tRemove name on <channel>%r
%t%t%t%t%tThese will not affect channels set%r
%t%t%t%t%tNO_ALIASES%r
%r
+com/credits%t%t%t-%tShow credits for X-Channels%r
[repeat(=,75)]
-
&text_wizhelp X-Channels Data Storage=
[repeat(=,75)]
[center(X-Channels 2.0 Wizard Help,75)]%r
[repeat(=,75)]%r
+com/addchan <channel>=<comment>-%tSet up a new channel, with comment.%r
+com/rmchan <channel>%t%t-%tRemove a channel.%r
+com/lockchan <channel>=<lock>%t-%tSet the lock function on a channel.%r
+com/blockchan <channel>=<lock>%t-%tSet the broadcast lock function on%r
%t%t%t%t%ta channel.%r
+com/showlock <channel>=<lock>%t-%tDisplay the lock function on a%r
%t%t%t%t%tchannel.%r
+com/showblock <channel>=<lock>%t-%tDisplay the broadcast lock%r
%t%t%t%t%tfunction on a channel.%r
+com/set <channel>=\[!\]<flag>%t-%tSet/clear a flag on a channel.%r
+com/flush <dbref>%t%t-%tRemove a dbref from all channels%r
%t%t%t%t%tat once%r
[repeat(=,75)]
-
&text_credits X-Channels Data Storage=
[repeat(=,75)]%r
[center(X-Channels 2.0 Credits,75)]%r
[repeat(=,75)]%r
Version 1.0%t-%tAmethyst @ Mountain Cauldron%r
%tWritten by Amethyst @ Mountain Cauldron, based on a combination of%r
%tinterfaces from Ashen-Shugar's com system, and Two Moons's com%R
%tsystem.%r
%r
Version 1.1%t-%tAmethyst @ Mountain Cauldron%r
%tAdded this credits/history file.%r
%tAdded +com/showlock%r
%tAdded suite of commands for locking broadcast ability on a%r
%t%tLIMITED_BROADCAST channel.%r
%r
Version 1.2%t-%tAmethyst @ Mountain Cauldron%r
%tFixed a bug in +com/leave that prevented leave-messages%r
%tAdded error checking to +com/addalias and +com/rmalias%r
%tAdded error checking to +com/setname and +com/rmname%r
%tFixed a bug in +com/addalias%r
%r
Version 1.3%t-%tAmethyst @ Mountain Cauldron%r
%tSplit admin and user commands into separate objects%r
%r
Version 2.0%t-%tLucifer @ Prairie Fire%r
%tChanged all @com commands to +com%r
%tAdded +com/flush%r
%tAdded | (name post-pose) format to channel messages%r
%tAdded ~ (emit) format to channel messages%r
%tAdded NO_EMITS and PARANOID flags%r
%r
Version 2.1%t-%tLucifer @ Prairie Fire%r
%tAdded +com/version%r
%t+com/flush now accepts playernames/aliases as well as dbrefs%r
%tFixed a typo in +com/blockchan%r
%tDark-flagged players are no longer seen joining/leaving a channel%r
[repeat(=,75)]
-
&text_version X-Channels Data Storage=
[repeat(=,75)]%r
X-Channels version 2.1%r
[repeat(=,75)]
-
