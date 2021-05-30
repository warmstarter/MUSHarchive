( notify_code: Send a message to a player )
( ------------------------------------------------ )
( The Main Program: send a message just like page does )
var who
var msg
var player
var msgstr
: parse
  read who !
  read msg !
  ( set the player object )
  who @ .pmatch player !
  player @ int -1 = if
    me @ "No such player: " who @ strcat notify exit
    then
  ( check for being connected )
  player @ awake? not if
    me @ "That person is not connected." notify exit
    then

  ( set the message )
  msg @ "." strcmp 0 = if
    "You sense that "
    me @ name strcat
    " is looking for you in " strcat
    me @ location name strcat
    "." strcat
    msgstr !
    player @ msgstr @ notify
    
    me @ "Your message has been sent." notify
  else
    me @ name " " strcat msg @ strcat msgstr !
    player @ msgstr @ notify
    me @
    "You send \""
msgstr @ strcat
"\" to player " strcat
player @ name strcat
"." strcat
notify
  then
;
