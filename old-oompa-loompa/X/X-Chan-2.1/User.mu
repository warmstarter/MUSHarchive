# And now, without further ado, the main code for non-administrative commands.
#
&cmd_com_list X-Channels 2.0 User Commands=$+com/list:
@pemit %#=%b%bChannel[repeat(%b,18)]Comment[repeat(%b,36)]Flags
%r[repeat(=,75)]
[
iter(
  filter(
    fnc_map_visible,
    v(list_channels)
  ),
  %r
  [switch(match(v(chan_##_list),%#),0,%b,*)]
  %b
  ##
  [setq(0,v(chan_##_comment))]
  [repeat(%b,sub(25,strlen(##)))]
  %q0
  [repeat(%b,sub(43,strlen(%q0)))]
  [u(fnc_showflags,##)]
)
]
%r[repeat(=,75)]
-
&cmd_com_join X-Channels 2.0 User Commands=$+com/join *:
@wait me={

@switch/first 
[setq(0,u(fnc_match_channel,%0,%#))]
[gt(match(v(list_channels),%q0),0)]
[u(fnc_chanok,%q0,%#)]
[eq(match(v(chan_%q0_list),%#),0)]
=
0??,
{
  @pemit %#=ERROR: '%0' does not match any known channel or alias.;
  @notify me
},
10?,
{
  @pemit %#=ERROR: Permission denied.;
  @notify me
},
110,
{
  @pemit %#=ERROR: You are already on that channel.;
  @notify me
},
{
  &chan_%q0_list [parent(me)]=[setunion(v(chan_%q0_list),%#)];
  @pemit/list filter(fnc_announce,v(chan_%q0_list))=
    \[%q0\] [u(fnc_channame,%#,%q0)] has joined this channel.;
  @notify me
}

}
-
&cmd_com_+ X-Channels 2.0 User Commands=$+com +*:
@wait me={

@switch/first 
[setq(0,u(fnc_match_channel,%0,%#))]
[gt(match(v(list_channels),%q0),0)]
[u(fnc_chanok,%q0,%#)]
[eq(match(v(chan_%q0_list),%#),0)]
=
0??,
{
  @pemit %#=ERROR: '%0' does not match any known channel or alias.;
  @notify me
},
10?,
{
  @pemit %#=ERROR: Permission denied.;
  @notify me
},
110,
{
  @pemit %#=ERROR: You are already on that channel.;
  @notify me
},
{
  &chan_%q0_list [parent(me)]=[setunion(v(chan_%q0_list),%#)];
  @pemit/list filter(fnc_announce,v(chan_%q0_list))=
    \[%q0\] [u(fnc_channame,%#,%q0)] has joined this channel.;
  @notify me
}

}
-
&cmd_com_leave X-Channels 2.0 User Commands=$+com/leave *:
@wait me={

@switch/first 
[setq(0,u(fnc_match_channel,%0,%#))]
[gt(match(v(list_channels),%q0),0)]
[gt(match(v(chan_%q0_list),%#),0)]
=
0?,
{
  @pemit %#=ERROR: '%0' does not match any known channel or alias.;
  @notify me
},
10,
{
  @pemit %#=ERROR: You are not on that channel.;
  @notify me
},
{
  @pemit/list filter(fnc_announce,v(chan_%q0_list))=
    \[%q0\] [u(fnc_channame,%#,%q0)] has left this channel.;
  &chan_%q0_list [parent(me)]=[setdiff(v(chan_%q0_list),%#)];
  @notify me
}

}
-
&cmd_com_- X-Channels 2.0 User Commands=$+com -*:
@wait me={

@switch/first 
[setq(0,u(fnc_match_channel,%0,%#))]
[gt(match(v(list_channels),%q0),0)]
[gt(match(v(chan_%q0_list),%#),0)]
=
0?,
{
  @pemit %#=ERROR: '%0' does not match any known channel or alias.;
  @notify me
},
10,
{
  @pemit %#=ERROR: You are not on that channel.;
  @notify me
},
{
  @pemit/list filter(fnc_announce,v(chan_%q0_list))=
    \[%q0\] [u(fnc_channame,%#,%q0)] has left this channel.;
  &chan_%q0_list [parent(me)]=[setdiff(v(chan_%q0_list),%#)];
  @notify me
}

}
-
&cmd_com_who X-Channels 2.0 User Commands=$+com/who *:
@wait me={

@switch/first 
[setq(0,u(fnc_match_channel,%0,%#))]
[gt(match(v(list_channels),%q0),0)]
[gt(match(v(chan_%q0_list),%#),0)]
=
0?,
{
  @pemit %#=ERROR: '%0' does not match any known channel or alias.;
  @notify me
},
10,
{
  @pemit %#=ERROR: You are not on that channel.;
  @notify me
},
{
  @pemit %#=People on channel %q0:%r
  [repeat(=,75)]%r
    [u(fnc_who_[switch(u(fnc_canseedark,%#),0,no,1,)]dark,%q0)]%r
  [repeat(=,75)];
  @notify me
}

}
-
&cmd_equals X-Channels 2.0 User Commands=$=* ?*:
@wait me={

@pemit/list 
[setq(0,
  [setq(1,u(fnc_match_channel,%0,%#))]
  [gt(match(v(list_channels),%q1),0)]
  [gt(match(v(chan_%q1_list),%#),0)]
  [or(not(u(fnc_chanflag,%q1,LIMITED_BROADCAST)),u(fnc_broadcastok,%q1,%#))]
)]
[setq(2,ulocal(fnc_channame,%#,%q1))]
[switch(%q0,111,v(chan_%q1_list),%#)]
=
[switch(%q0,
  0??,
  {ERROR: '%0' does not match any known channel or alias.},
  10?,
  {ERROR: You are not on that channel.},
  110,
  {ERROR: You are not allowed to broadcast on that channel.},
  {
    \\\[%q1\\\]%b
    [switch(%1,
      :,
      %q2%b%2,
      ;,
      %q2%2,
      |,
      %2 (%q2),
      ~,
      [switch(0,
        u(fnc_chanflag,%q1,NO_EMITS),
        %2,
        %2 (%q2)
      )],
      ",
      %q2%bsays%b"%2",
      %q2%bsays%b"%1%2"
    )]
  }
)];
@notify me
}
-
&cmd_com_addalias X-Channels 2.0 User Commands=$+com/addalias * *:
@switch/first 0=
[setq(0,u(fnc_match_channel,%1,%#))]
[match(v(list_channels),%q0)],
{
  @pemit %#=There is no '%1' channel.
},
{
  &channel_%0 %#=%q0;
  @pemit %#=You set your '%0' channel alias to match '%q0'.
}
-
&cmd_com_rmalias X-Channels 2.0 User Commands=$+com/rmalias *:
@switch/first 0=
[strlen(get(%#/channel_%0))],
{
  @pemit %#=You don't have a '%0' channel alias.
},
{
  &channel_%0 %#=;
  @pemit %#=You remove your '%0' channel alias.
}
-
&cmd_com_aliases X-Channels 2.0 User Commands=$+com/aliases:
@pemit %#=
Alias[repeat(%b,10)]Channel%r
[repeat(=,75)]
[iter(
  lattr(%#/channel_*),
  %r
  [setq(0,delete(##,0,8))]
  [capstr(lcstr(%q0))]
  [repeat(%b,sub(15,strlen(%q0)))]
  [u(fnc_match_channel,get(%#/##),%#)]
)]%r
[repeat(=,75)]
-
&cmd_com_setname X-Channels 2.0 User Commands=$+com/setname * *:
@switch/first 0=
[setq(0,u(fnc_match_channel,%0,%#))]
[match(v(list_channels),%q0)],
{
  @pemit %#=There is no '%0' channel.
},
{
  &chanalias_%0 %#=%1;
  @pemit %#=You set your name on the %0 channel to %1.
}
-
&cmd_com_rmname X-Channels 2.0 User Commands=$+com/rmname *:
@switch/first 0=
[strlen(get(%0/chanalias_%0))],
{
  @pemit %#=You have no '%0' channel alias.
},
{
  &chanalias_%0 %#=;
  @pemit %#=You remove your name on the %0 channel.
}
-
&cmd_com_flags X-Channels 2.0 User Commands=$+com/flags:
@pemit %#=Channel flags:
[
iter(
  v(list_chanflags),
  %r
  ([u(fnc_abbrevflag,##)])%b
  ##
  [repeat(%b,sub(25,strlen(##)))]
  [v(comment_##)]
)
]
-
&cmd_com_help X-Channels 2.0 User Commands=$+com/help:
@pemit %#=[u(text_help)]
-
&cmd_com_credits X-Channels 2.0 User Commands=$+com/credits:
@pemit %#=[u(text_credits)]
-
&cmd_com_version X-Channels 2.0 User Commands=$+com/version:
@pemit %#=[u(text_version)]
-
