Author: Philip Mak <pmak@aaanime.net>

This is an IRC <-> MUSH gateway. You need PennMUSH and TinyFugue. This is 
unsupported open-source software. If you make improvements to it, you must 
send me a copy.

Your TinyFugue client should already define the worlds "irc" and
"sa_mush". The "sa_mush" world should have auto-login. Create the channel
"Chat" on your MUSH. Also create the IRC Commands object (code in 
irc.mush) and put it in #2.

To start the bot, load TinyFugue (preferably inside 'screen' so it will
stay running after you log off, and you can reattach) and /load start.tf.
