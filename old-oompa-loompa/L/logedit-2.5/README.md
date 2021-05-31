Logedit Installation

1.	Unpack the archive. You must've done that, because you're reading
this!

2.	Edit logedit.h (and possibly logedit.c)

	You'll want to set the CONFDIR and CONFFILE variables to
something appropriate for you. I put my configuration stuff in 
~/.logeditrc. (But use a full pathname for CONFDIR). A sample
configuration file, sample.logeditrc, is included.
	
	If you tend to frequent TinyMUSH 2.0, you might want to edit
the source code (logedit.c), uncomment the /* #define TINYMUSH */
If you are a fan of PennMUSH, it's already set up for you.

	SunOS users should uncomment the /* #define SUNOS */ in logedit.h,
and compile with gcc. (gcc -o logedit logedit.c) These modifications
were written by Christian Scarborough.

	If your system uses some different time functions than mine,
you'll have to hack a bit or remove the time-stamp at the beginning
of logs. Same with regexp evaluation functions.

3.	Copy Makefile.dist to Makefile (cp Makefile.dist Makefile).
	Edit Makefile for your system. Logedit is written in ANSI
        C, so try to specify an ANSI-compliant compiler for CC.
	GNU CC (gcc) is great if you've got it.

        Read the information in the Makefile about the curses library
        options, and see you're interested. An easy way to find out
        which (if any) curses libraries you have is to try:
        ls /usr/include/curs*.h
        and see what comes up.

4. 	Run 'make'. This will compile logedit.
	After compiling, you can save disk space by stripping the executable.
        Try: strip logedit

5.	Run 'make install' if you want to install logedit and the man page
	somewhere.

6.      Read the enclosed manual page for information about using logedit,
	setting up a configuration file, etc. The man page is included
	in both unformatted (logedit.1) and clean-formatted
	(logedit.man) versions, in case you choose to install it
	somewhere.

