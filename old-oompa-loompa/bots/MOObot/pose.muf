Replacement for pose ':' )
( ------------------------------------------------ )
send a message just like pose does )
var msg
var msgstr
parse
  read msg !
  ( set the message )
  me @ name " " strcat msg @ strcat msgstr !
  loc @ #-1 msgstr @ notify_except
;
