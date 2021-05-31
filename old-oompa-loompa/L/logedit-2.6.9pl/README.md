Logedit v2.6pl Installation

1.	Unpack the archive. You must've done that, because you're reading
	this!

2.	Type 'sh inst.sh'

	This shell script will try to determine the location of perl on
	your system, and will ask you to verify the location (or give
	one if it can't find it). It will then run the 'inst.pl'
	installation script which will ask you where you want the
	script and manpage installed, and do the installation.

	If for some reason the script doesn't work, all you really
	*need* to do is copy logedit.dist over to be called 'logedit',
	make sure the first line (#!...) points to perl on your system,
	find the #$unix=1; line and uncomment it if you're on a unix
	system, and make it executable.

3.      Read the manual page for information about using logedit,
	setting up a configuration file, etc. The logedit script
	is its own man page (i.e., you can nroff -man logedit to read
	it), and the installation routine will give you the option of
        installing it either unformatted or formatted.
	A separate copy of the formatted man page is included as
	"logedit.man" in case you don't want to install the man page,
	but want a copy around to read.

4.	That's it! Enjoy, and please send comments/bug reports/ideas!
