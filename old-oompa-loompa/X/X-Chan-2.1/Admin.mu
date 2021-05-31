# Now we get into the administrative commands.
#
@lock/use X-Channels 2.0 Admin Commands=FNC_ISADMINISTRATOR/1
-
&cmd_com_addchan X-Channels 2.0 Admin Commands=$+com/addchan *=*:
@wait me={

@switch/first 0=
[not(gt(match(v(list_channels),%0),0))],
{
  @pemit %#=ERROR: That channel already exists.;
  @notify me
},
{
  &list_channels [parent(me)]=[setunion(v(list_channels),%0)];
  &chan_%0_comment [parent(me)]=%1;
  @pemit %#=Channel '%0' added with comment '%1';
  @notify me
}

}
-
&cmd_com_rmchan X-Channels 2.0 Admin Commands=$+com/rmchan *:
@wait me={

@switch/first 0=
[gt(match(v(list_channels),%0),0)]
[setq(0,u(fnc_match_channel,%0,%#))],
{
  @pemit %#=ERROR: That channel does not exist.;
  @notify me
},
{
  &list_channels [parent(me)]=[setdiff(v(list_channels),%q0)];
  @wipe [parent(me)]/chan_%0_*;
  @pemit %#=Channel '%0' removed.;
  @notify me
}

}
-
&cmd_com_lockchan X-Channels 2.0 Admin Commands=$+com/lockchan *=*:
@wait me={

@switch/first 0=
[gt(match(v(list_channels),%0),0)],
{
  @pemit %#=ERROR: That channel does not exist.;
  @notify me
},
{
  &chan_%0_lock [parent(me)]=%1;
  @pemit %#=Lock for channel '%0' set to %1;
  @notify me
}

}
-
&cmd_com_blockchan X-Channels 2.0 Admin Commands=$+com/blockchan *=*:
@wait me={

@switch/first 0=
[gt(match(v(list_channels),%0),0)],
{
  @pemit %#=ERROR: That channel does not exist.;
  @notify me
},
{
  &chan_%0_broadcastlock [parent(me)]=%1;
  @pemit %#=Broadcast lock for channel '%0' set to %1;
  @notify me
}

}
-
&cmd_com_showlock X-Channels 2.0 Admin Commands=$+com/showlock *:
@wait me={

@pemit %#=
  [switch(
    [gt(match(v(list_channels),%0),0)],
    0,
      {ERROR: That channel does not exist.},
      {%0's lock is: [v(chan_%0_lock)]}
  )];
@notify me

}
-
&cmd_com_showblock X-Channels 2.0 Admin Commands=$+com/showblock *:
@wait me={

@pemit %#=
  [switch(
    [gt(match(v(list_channels),%0),0)],
    0,
      {ERROR: That channel does not exist.},
      {%0's broadcast lock is: [v(chan_%0_broadcastlock)]}
  )];
@notify me

}
-
&cmd_com_set X-Channels 2.0 Admin Commands=$+com/set *=?*:
@wait me={

@switch/first 0=
[gt(match(v(list_channels),%0),0)],
{
  @pemit %#=ERROR: That channel does not exist.;
  @notify me
},
{
  &chan_%0_flags [parent(me)]=
    [setq(0,switch(comp(%1,!),0,0,1))]
    [setq(1,v(list_chanflags))]
    [setq(2,extract(%q1,match(%q1,switch(%q0,0,%2,%1%2)),1))]
    [add(
      v(chan_%0_flags),
      [switch(
      [u(fnc_chanflag,%0,%q2)]%q0,
      01,
        [v(chanflag_%q2)],
      00,
        0,
      11,
        0,
      10,
        -[v(chanflag_%q2)]
      )]
    )];
  @pemit %#=Flag %q2 [switch(%q0,0,cleared from,set on)] channel %0.;
  @notify me
}

}
-
&cmd_com_flush X-Channels 2.0 Admin Commands=$+com/flush *:
@wait me={

@switch/first 0=
[setq(0,locate(%#,*%0,p))][isdbref(%q0)],
{
  @pemit %#=That isn't a valid playername or dbref.;
  @notify me
},
{
  @dolist v(list_channels)=
    {
      &chan_##_list [parent(me)]=[setdiff(v(chan_##_list),%q0)]
    };
  @dolist 1={@pemit %#=[name(%q0)] flushed from the +com system.;@notify me}
}

}
-
&cmd_com_wizhelp X-Channels 2.0 Admin Commands=$+com/wizhelp:
@pemit %#=[u(text_wizhelp)]
-
