Webster the Spellbot (Version 4.1.3, Perl)
	   Rewritten from 3.4 by Raevnos (Shawn Wagner - raevnos@pennmush.org)
  	 (No warranty granted or implied.)

Webster is a MUSH robot which provides an interface to the Unix
'ispell' command (for spell-checking), and to the dict.org dictionary
server.

File list:
	webster		The robot code itself
	addspell	Useful utility for local dictionaries
	spellobj.msh	A MUSH master room object to use as
			a safe interface to Webster.
        README          Well, duh.

INSTALLATION & UPGRADING

0. You must have perl 5.005 or greater installed to use Webster 4.1.
   See http://www.perl.com. Unlike in earlier versions, all required 
   non-standard modules are included. If you've used an older version of
   Webster, please follow the installation instructions, because 4.1 has
   new softcode for both +spell and +define.

1. From the account

* Edit webster to set parameters. In particular, don't set Webster's password
  ($web_pass) to an English word. Most of the parameters are
  pretty self-explanatory. The very first line of webster should
  point to perl on your system (Usually /usr/bin/perl or /usr/local/bin/perl).
* Be sure to chmod +x webster.
* Create your local dictionary by using the 'touch' command:
	touch local.dict

2. From the MUSH

* Create a character named Webster, with the appropriate password
* Create a room for Webster, and put it there. Link it there. Set the
  home FLOATING, and don't make exits into it. Webster is operated
  solely by @pemit's and/or pages. Then, @power it pemit_all and idle,
  and, if you wish, hide.
* Spellobj.msh is some uploadable mush code to be used on a master room
  global commands object. It creates the +spell command which
  can be used as +spell <text> or +spell <obj>/<attrib> and the +define
  command which can be used as +define <words>. There are no versions that
  require specific unformatters, like in earlier versions. <rant> If you want
  to use formatters when writing code, fine. Just don't distribute it still
  formatted.</rant> You can just /quote it directly in, or copy and paste
  the relevant bits.
* Page-lock Webster to you and the object that the code from spellobj.msh is
  on. This prevents other players from somehow forcing Webster to disconnect,
  or other possible badness.

3. From the account again:

* If upgrading, shut down the currently running Webster by something like
        @pemit *webster=GOODBYE <PASSWORD>

* Run webster:
	From (t)csh, ./webster >& web.log &
  	From bash, ./webster > web.log 2>&1 &

* To reload the local dictionary after adding words to it:
       kill -USR1 <webster's pid>

USING WEBSTER IN THE MUSH

Once Webster is connected, anything paged to Webster will be
spellchecked or defined.  Other text will be ignored, unless it's
'GOODBYE <password>', in which case, Webster will shut down and
disconnect. Use this instead of @boot.

Webster assumes that the player paging it is to get the answer, and that
the first word of the page is the name of an attribute. This is useful
with the +spell <obj>/* command, so that Webster can return more useful
information about spelling errors. This is also why I recommend using 
+spell and page-locking webster, rather than allowing players to page
directly - Webster is easily spoofed. (In fact, +spell works by spoofing
Webster).

THE ADDSPELL SCRIPT

The addspell script is a convenient way to add words to your local
dictionary, which, according to the ispell man page, should be a sorted
list of words, one per line (you don't have to add plurals, since ispell
does derivations). Edit it to configure it for your setup.

Syntax: addspell <word>
Addspell will create the local dictionary (edit it to set the filename)
if it doesn't exist, and will add the word if it's not already in the
dictionary, and resort the dictionary.

CHANGES
4.1.4
Support for MUX.

4.1.3
Updated Net::Dict module to the latest version.
Escaped some more characters in definitions.
Words with lots of definitions will no longer get truncated due to buffer limits
 quite as easily.
Multi-word phrases can be +defined by surrounding them with quotes
 (+define "once again")
Cross-references in definitions are highlighted to stand out better.

4.1.2
Fixed a problem with +define and perl 5.6

4.1.1
Fixed problem with Webster disconnecting if it can't connect
 to the dictionary server.
+defining a word that isn't in the dictionary does better reporting
of that fact.
Better handling of ispell metacharacters
Better handling of regexp metacharacters in Webster's password.
Removed the KEEP_PROCESS option - it's always on now.
Use IO::File instead of Filehandle.
Changed maintainer's email adress again. :)

4.1
Added +define dictionary lookups.
Added SIGUSR1 handler to reload the local dictionary.
Re-wrote the +spell softcode.
Changed maintainer's email address.

4.0.3
Got rid of zombie processes when $KEEP_PROCESS is set to 0.
Fixed a bug that could hang webster (Javelin)
Started using PRCS to keep track of revisions.

4.0.2
Bugfixes involving local dictionarys
Major bug fix involving what appears to be a bug in IPC:Open2. Now you won't
have words from previous +spells showing up instead of what you ran. I hope.

4.0.1
Bugfix for when $KEEP_PROCESS is set to 0.

4.0
Uses ispell -a instead of spell, and return suggestions for
 misspelled words. Very popular feature on my MUSH.
No longer uses temporary files.
No longer starts a new spell process every time +spell is
 used - only one ispell process throughout the entire life
 of the program. The old behavior can still be enabled, though.
Keeps the logfile open throughout the entire life of the
 program, instead of opening, writing and closing each time
 +spell has misspelled words. Old behavior can still be
 enabled.
Logging can be disabled.
Reports the results via @pemit, not page.
Minor changes to make the code more perl-ish: Fewer ()'s,
Webster no longer disconnects when it sees it's password
 mixed with other words. The proper format is:
        @pemit/s *Webster=GOODBYE <PASSWORD>
Lots of rewritten code.


FUTURE RELEASES

Unless more bugs appear, this is the last release of Webster 4.  I'm
re-writing Webster 5 from scratch, including more features. However,
it will be in a different language (Objective Caml
(http://www.ocaml.org)), so I will continue to make bug-fix releases
of Webster 4 for people who don't want to switch. No new features are
planned for Webster 4, though sometimes I lie about that.

