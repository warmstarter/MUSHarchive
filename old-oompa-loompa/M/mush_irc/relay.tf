; relay.tf v1.1
; by Philip Mak
; Sunday, March 24, 2002
;
; Relay messages between an IRC channel and a PennMUSH channel

/def sv = /echo Use a text editor to edit the script instead of saving it.
/def flush = /fg -q -<%;/fg -q -<
/def rel = /flush%;/load start.tf
/def -p1 -ag -mglob -h'BACKGROUND ' 

; Routines to stay connect and stay connected

/def -p0 -mglob -h'CONNECT irc' conn_irc = NICK MUSH%;USER sa_mush sa_mush sa_mush :/msg MUSH help%;/repeat -3 1 /send -wirc JOIN #shoujoai
/def -p0 -mglob -h'CONFAIL irc*' confail_irc = /repeat -3 1 /world irc
/def -p0 -mglob -h'DISCONNECT irc*' discon_irc = /world irc%;/send -wsa_bot @cemit/noeval/noisy Chat=!Connection to IRC lost. Attempting to reestablish.%;/repeat -3 1 /world irc
/def -p2 -w'irc' -mglob -t'MUSH!* JOIN :#shoujoai' conned_irc = /send -wsa_bot @cemit/noisy/noeval Chat=!Connection to IRC reestablished.

/def -p1 -w'irc' -mglob -t'PING *' ping_irc = PONG %{2}
/def tick_action = /send -wirc PING :keepalive
/def tick_warn =

/def -p0 -mglob -h'CONFAIL sa_bot*' confail_mush = /repeat -3 1 /world sa_bot
/def -p0 -mglob -h'DISCONNECT sa_bot*' discon_mush = /world sa_bot%;/send -wirc PRIVMSG #shoujoai :!Connection to MUSH lost. Attempting to reestablish.%;/repeat -3 1 /world sa_bot

; Basic message relay routines

/def -p1 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ PRIVMSG #shoujoai :(.*)$' say_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 says, "%P2"
/def -p2 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ PRIVMSG #shoujoai :ACTION (.*)$' me_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 %P2
/def -p1 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ JOIN :#shoujoai' join_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 has joined this channel.
/def -p1 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ PART #shoujoai' part_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 has left this channel.
/def -p2 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ PART #shoujoai :(.*)' part2_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 has left this channel (%P2).
/def -p1 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ QUIT' quit_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 has disconnected.
/def -p2 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ QUIT :(.*)' quit2_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P1 has disconnected (%P2).
/def -p1 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ NICK :(.*)' nick_irc = /send -wsa_bot @cemit/noeval/noisy Chat=!%P1 is now known as %P2.
/def -p1 -w'irc' -mregexp -t'^:([^!]+)[^ ]+ KICK #shoujoai ([^ ]+) :(.*)$' kick_irc = /send -wsa_bot @cemit/noeval/noisy Chat=<%P2 was kicked by %P1 (%P3)

/def -p1 -w'sa_bot' -mglob -t'<Chat> *' mush = /send -wirc PRIVMSG #shoujoai :%{-1}
/def -p2 -w'sa_bot' -mglob -t'<Chat> <*' mush2 
/def -p2 -w'sa_bot' -mglob -t'<Chat> !*' mush3 

; MUSH to IRC response routines
; Every /enqueue must have a corresponding /dequeue.

/def -p1 -w'sa_bot' -mglob -t'NAMES *' namesq_mush = /enqueue %{2}%;/send -wirc NAMES #shoujoai
/def -p1 -w'sa_bot' -mglob -t'WHOIS * *' whoisq_mush = /enqueue %{2}%;/send -wirc WHOIS %{3}
/def -p1 -w'sa_bot' -mglob -t'MSG * * *' msgq_mush = /enqueue %{2}%;/send -wirc PRIVMSG %{4} :<%{3}> %{-4}

/def -p1 -w'irc' -mregexp -t'^[^ ]+ 353 [^ ]+ = #[^ ]+ :(.*)' names_irc = /send -wsa_bot @pemit/silent $(/dequeue)=[ansi(g,IRC Members of channel <Chat> are:)]%%r[lit(%P1)]
/def -p1 -w'irc' -mregexp -t'^[^ ]+ 401 [^ ]+ (.*)' nosuchnick_irc = /send -wsa_bot @pemit/silent $(/dequeue)=[ansi(hr,ERROR:)] [lit(%P1)]
/def -p1 -w'irc' -mregexp -t'^[^ ]+ (301|311|312|313|317|319) [^ ]+ (.*)' whois_irc = /send -wsa_bot @pemit/silent $(/car %{queue})=[ansi(hg,WHOIS:)] [lit(%P2)]
/def -p1 -w'irc' -mregexp -t'^[^ ]+ 318 [^ ]+ (.*)' whois2_irc = /dequeue

; IRC to MUSH response routines
; Every /enqueue must have a corresponding /dequeue.

/def -p0 -w'irc' -mregexp -t'^:([^!]+)![^ ]+ PRIVMSG MUSH' default_irc = PRIVMSG %P1 :Unknown command. Try 'help'.
/def -p1 -w'irc' -mregexp -t'^:([^!]+)![^ ]+ PRIVMSG MUSH :help$' help_irc = PRIVMSG %P1 :This bot is a gateway that allows you to communicate with players on Shoujo-Ai MUSH (http://www.shoujoai.com/mush/). Commands: 'who', 'names', 'msg <player> <message>'
/def -p1 -w'irc' -mregexp -t'^:([^!]+)![^ ]+ PRIVMSG MUSH :msg ([^ ]+) (.*)' msgq_irc = /send -wsa_bot @switch num(*%P2)=#-1, {think MSGREPLY %P1 Non-existent player name: %P2}, {@switch conn(%P2)=-1, {think MSGREPLY %P1 Player [name(*%P2)] is not logged on.}, {@pemit/silent/noeval *%P2=%P1@IRC pages: %P3;think MSGREPLY %P1 Message sent to %P2.}}
/def -p1 -w'irc' -mregexp -t'^:([^!]+)![^ ]+ PRIVMSG MUSH :who' whoq_irc = /send -wsa_bot think MSGREPLY %P1 Connected players on MUSH:%;/send -wsa_bot think MSGREPLY %p1 [iter(lwho(),name(##))]
/def -p1 -w'irc' -mregexp -t'^:([^!]+)![^ ]+ PRIVMSG MUSH :names' namesq_irc = /send -wsa_bot think MSGREPLY %P1 Connected players on MUSH chat channel:%;/send -wsa_bot think MSGREPLY %p1 [iter(cwho(Chat),name(##))]
; /def -p1 -w'irc' -mregexp -t'^:([^!]+)![^ ]+ PRIVMSG MUSH :kick ([^ ]+)' kickq_irc = /def -n1 -p10 -w'irc' -mregexp -t':[^ ]+ 353 MUSH = #[^ ]+ :([[^ ]+ ]*)(@?)%{P1}' kick2_irc=/if ({P2} =~ '@') /send -wsa_bot @cemit/noeval/noisy Chat=%P1 boots %P2 from the channel.%%;/send -wsa_bot @chan/off Chat=%P2%%;/else PRIVMSG %P1 :You're not channel operator (%%P0)%%;/endif%;NAMES #shoujoai%;/list kick2_irc

/def -p1 -w'sa_bot' -mglob -t'MSGREPLY * *' msgreply_mush = /send -wirc PRIVMSG %{2} %{-2}
