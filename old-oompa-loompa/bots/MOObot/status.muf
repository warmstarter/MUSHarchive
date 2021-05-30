( Status: dump room number, name, desc, exits, and contents )
: show_pennies
me @ pennies 3000 < if me @ 5000 addpennies then
me @ pennies 9000 > if me @ -1000 addpennies then
me @ "=penn=" me @ pennies intostr strcat notify
;
: show_room_id
me @ "=numb=" loc @ int intostr strcat notify
;
: show_room_is_player
me @ "=ispl=" loc @ player? if
    "1"
  else
    "0"
  then
  strcat notify
;
: show_room_name
me @ "=name=" loc @ name strcat notify
;
: show_room_desc
me @ "=desc=" loc @ desc strcat notify
;
: show_exit_list
dup int -1 = if pop exit then
dup getlink room? if
dup dup name swap getlink int intostr swap " " swap strcat strcat
"=exit=" swap strcat me @ swap notify
then
next show_exit_list
;
: show_room_exits
loc @ exits show_exit_list
;
: show_players_present
dup int -1 = if pop exit then
dup player? if
dup dup int intostr swap name "=" swap strcat strcat
"=plyr=" swap strcat me @ swap notify then
next show_players_present
;
: show_room_players
me @ "=players=" notify
loc @ int 69 = if else
    loc @ contents show_players_present
then
;
: show_things_present
dup int -1 = if pop exit then
dup thing? if dup name "=thng=" swap strcat me @ swap notify then
next show_things_present
;
: show_room_contents
me @ "=contents=" notify
loc @ int 69 = if else
    loc @ contents show_things_present
then
;
: status
me @ "!jump" set
me @ "!abode" set
me @ "!haven" set
me @ "!link" set
( Reset Julias state after each move )
show_pennies
show_room_id
show_room_is_player
show_room_name
show_room_desc
( show_room_exits )
show_room_contents
show_room_players
;
