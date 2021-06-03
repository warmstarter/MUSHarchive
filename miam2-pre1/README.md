
 *  If there already is a decompiles.mu and a creates.mu in the archive you
 .  received, you may ignore the first couple of lines in the above
 .  instructions, I already did them for you.
 .
 .


 *
 .  /quote 'myfile   is how you tell the tinyFugue MU client to read the
 .  file "myfile" from disk and slowly feed it to the MU line by line, as
 .  if you'd typed in the contents of the file.
 .  If you use something other than tinyFugue (tf), you'll have to find out
 .  how to replay files in that client.
 .
 .


SrcMU:  the originating MU (using tinyFugue)
DstMU:  the receiving MU, where data are replicated (using tinyFugue)
Shell:  any ol' 'ix shell




Shell: for i in *.c; do j=`basename $i .c`; echo "compiling $j..."; gcc -o $j $i; done

  Build the tools using the C-compiler (example using gcc):

SrcMU: @search                                        ...giving search.log

  Find all relevant objects. (Usually provided by the MIAM2 team)
  The resulting log should be named "search.log".  Don't forget
  to turn on logging, and turn off word-wrap in logs!

Shell: 2decompiles < search.log > decompiles.mu

  Create "@decompile" instructions for all the relevent objects.

SrcMU: /quote 'decompiles.mu                          ...giving decompiled.log

  Feed the @decompile commands back to the MUSH.
  This will give us the MUSH-code for all the objects
  we're interested in; please log it to a file called
  "decompiled.log"

Shell: 2creates < search.log > creates.mu

  Create "@create" instructions for all the relevant objects.

DstMU: /quote 'creates.mu                             ...giving creates.log

  Feed the @create commands to the MUSH. Make sure you're
  in the "new" MUSH this time!
  This will create the required objects in the new MUSH
  and tell us the object-numbers ("db-refs") for the new
  objects.
  Log the results as "creates.log"

Shell: 2xrefs < creates.log > xrefs.DstMU

  Create xrefs. These are needed so the next step can fix
  the object-IDs ("dbrefs"). The dbrefs you got when creating
  the objects in your MUSH probably aren't the dbrefs I got
  when I created them here. If one of my objects mentions another
  one of my objects, they'll do it by mentioning the other object's
  dbref on *my* machine. That won't work for you since all the numbers
  have changed, so we'll change all "mentions" as well.  Don't think
  about this too hard. : )

Shell: xref < decompiled.log xrefs.DstMU > upload.mu

  Fix the dbrefs.  "decompiled.log" holds all the relevant objects
  like they are on my MUSH.  "xrefs.DstMU" holds a translation table
  saying which dbref on my MUSH is equivalent to which dbref on yours.
  The resulting "upload.mu" holds all the relevant objects like they
  should be in your MUSH (with all the dbrefs fixed).

DstMU: /quote 'upload.mu

  Feed "upload.mu" to the new MUSH; this will set all attributes on
  the objects you created earlier.
