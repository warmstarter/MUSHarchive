                         Javelin's IC logging system
                                Version 1.0

RATIONALE AND OPERATION

Running a judged RP MUSH can be a headache. Because judges can't always
be online to watch RP as it occurs, many MUSHes ask their players to
make logs of RP and to submit those logs to the RP judges or admin.
This allows the RP judges to reward players who participate in RP
as well as to keep track of ongoing plots.

However, the 'log-and-submit' system has some drawbacks. First,
not all clients are created equal. Some players may not have the
ability to log or the time (or software) to edit the log to remove
pages, chat channels, etc. Second, player-submitted logs can be
modified ex post by unscrupulous players. Finally, and perhaps most
importantly, the judges can be quickly deluged with RP logs. Overwhelmed
and unable to review every log, judges may soon stop reviewing logs
altogether.

At the now closed Riverworld MUSH, I developed an IC logging system
that addresses all three of these concerns.  The gist of the IC logging
system is (1) that the *server*, rather than the *player* maintains the
logs, and (2) that server-side scripts periodically sample the logs,
choosing those most likely to be noteworthy, and emailing no more than
a fixed number of logs (often 1) to each judge.

Here's how it works. Players use softcoded +log commands to turn
logging on or off for the current room (or object interior). The softcoded
global also uses an @adisconnect to turn off logging if the last person
present disconnects from the logged room, and checks all logging rooms
every 15 minutes in order to turn off logging if players are no longer
present.

When logging is activated or deactivated in a location, the server
announces this to anyone present in the location; in addition, when
logging is on at a location, any player who enters the location is
informed by the server that they are in a logged area. Internally,
logged rooms and objects have a LOGGING flag set on them which requires
royalty privs to set or unset.

Some areas should never be logged. Admin can set the OOC flag on these
areas. The server will refuse to activate logging in a room that
is flagged OOC. This flag provides players with further assurances
that their OOC conversation will not be recorded.

Once a location is being logged, any messages in that location generated
by players arriving or leaving, or the say, pose, semipose, @emit,
@remit, or @lemit commands are recorded in a server log file, ic.log,
along with the dbref of the speaker, the dbref of the location, and the
time. Chat channels, pages, whispers, @pemits, and other forms of
communication are not recorded, as these are assumed to be either OOC
or private.

So now we have a large log file containing, in chronological order,
anything said in any logged room. Because we know time, speaker, and
location, we can pretty accurately separate out distinct logs.
A perl script, process_iclog, reads the ic.log and separates it
into "conversations". It sorts the IC log by room and by time (i.e.,
chronologically within each room) and assumes that silences of
more than 15 minutes constitute the end of a conversation in a room.

process_iclog then tries to sort the conversations in order of
interestingness. What seemed to work well at Riverworld was
to compute the interestingness of a conversation by rolling some
10-sided dice for each conversation. The number of dice rolled
for a conversation was based on the sum of (1) the number of
different players involved in the conversation (more people is more
interesting), (2) the log (base 2) of the length of the conversation
in lines (longer conversations are more interesting, but at a decreasing
rate), and (3) the log (base 2) of the length in words of the
longest utterance in the conversation (conversations where people
speak at length are more interesting). Using dice introduces some
chance elements into the selection, while the formula above 
seemed to be very good at identifying interesting logs and 
largely ignoring monologues, short tests of the system, etc.

process_iclog produces a list of conversations from most interesting
to least. Another script, split-n-mail, then begins mailing these
conversations, in order, to the judges listed in a file of judge
addresses. It limits the number of logs sent to each judge to
a fixed number per judge (usually 1), and randomly orders the list
of judges each time it's called to keep the distribution of logs even.
It prepends each log with a paragraph suggesting how the judge should
use the log -- this, obviously, is game-specific.

Once a week, or at whatever interval works for you, you simply
run process_iclog on ic.log, feeding the results to split-n-mail,
and then clear ic.log.

This system works no matter what clients the players use. Players
can not modify the logs. And judges receive a manageable number
of submissions per period. Although the judges see only a sample
of the actual RP on the MUSH, those players who RP frequently, with
many others, and at length will tend to be more often represented in
the judged logs. The system does require that disk space be available
for the ic.log file, which, for security reasons might best be located
on its own partition.


INSTALLATION

1. Patch your PennMUSH server with the iclog_patch.172p14 patchfile.
   It contains instructions for how to do this.

2. Run 'make update' and agree to define IC_LOGGING in options.h

3. Recompile your server and @shutdown/reboot to enable the 
   logging server.

4. Upload the +log commands object. It's in the log_commands.mpp
   file in mpp format. You can get mpp at ftp.pennmush.org,
   or you can figure out what to do with it yourself.

5. Provide some appropriate player help. This I leave to you for now.

6. Install the files in the bin/ directory somewhere in your path.
   They should all be kept together. Check the first line of the
   two .pl files to be sure that the path to perl is right.
   Make these files executable.

7. Edit the split-n-mail.pl script to indicate where your 
   list of admin addresses will be found. Create this file. :)
   You probably also want to modify the mail that's sent with
   the logs, at the end of the split-n-mail.pl script.


DAILY USE

When you're ready to process the logs, copy the ic.log file
to a temporary directory, and clear the one in your
game directory with 'cp /dev/null ic.log'. On some systems, you
may have to do a kill -HUP to cause the logs to be reopened.

Then it's as easy as:
	process_iclog.pl < ic.log | split-n-mail.pl

Or, if you like to save the list of conversations sorted by
interestingness:
	process_iclog.pl < ic.log > ic.98-08-05
	split-n-mail.pl < ic.98-08-05


CREDITS

Written by Javelin (Alan Schwartz) for Riverworld MUSH.
You may use this under the same terms as the PennMUSH server source
code. Unfortunately, I don't have time to support this, so you're
on your own.

