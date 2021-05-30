This program is supposed to convert pretty Ascii pictures to MushCode.

You use it by typing ascii2mu <filename> where <filename> is the name of the file you want to convert
Or you can just run ascii2mu and input the filename at the helpful prompt.

It will do the following to your input file, and write it to <filename.msa>:

Change all spaces to %b's.  If there are more than 10 spaces in a row, it will change them to [space(xx)]
	(see help space() on your Mush.)
change all newlines, carriage returns, and stuff like that to %r's
change ['s to \['s
change ]'s to \]'s
change \'s to \\'s
change %'s to %%'s
keep every other character as is, unless there is more than 10 of the same character in a row.
	Then it will use [repeat(XX)]  (see help repeat() on your Mush)

It also supports ansi in the following way (see help ansi() on your mush):
If the program encounters [ansi(blah blah)] in your code, it will leave it alone.  Therefore, if you
want colored pictures, slap the [ansi()] function around the characters to color, just like you
would if you were typing it on a real mush.

If you are running Windows 95/98, stick the exe file in your windows/command directory to
be able to run it from any directory.  If you are running Windows 3.x or Dos, stick it in
your Dos directory.

If you convert a file to Mushcode, and run the program again on that file, you are going to have
a really messed up file.

This is a 16 bit program, so don't try running it on files that don't follow the DOS 8.3 naming scheme
Future things to implement:
	Strip trailing spaces off of a line
	Ignore ansi()



FAQ
---

Q: Why a FAQ?  Does such a dinky program warrant it?
A: Please, one question at a time. That's how FAQ's work.

Q: Does this program work on unix?
A: I don't know, I've been meaning to try that.  Obviously, I haven't yet.
   
	There is a a perl script which resembles this program, by Javelin       	<dunemush@pennmush.tinymush.org> which will run on unix. It can be found at
	ftp://ftp.pennmush.org/pub/PennMUSH/Scripts/ascii2mush.pl

	There's also a C source file at ftp://ftp.cis.upenn.edu/pub/lwl/amber/mtrans.c which will run
	on unix (and maybe Dos) which is similar to this program.  Original Lex code by T. E. Snider
	(Coral @ AmberMUSH; Misra @ DominionMUSH) 

	Note those programs are great, I wasn't trying to make a better one. I just felt like
	programming something.


Q: Does this program work on OS/2?
A: I don't know

Q: How do I use this program with TinyFugue?
A: I dunno, I don't use TinyFugue.

Q: How do I use this program with SimpleMu*?
A: Convert an ascii file as instructed at the beginning of the file. Open the corresponding *.mas
   file when you are done in Notepad.  Select all the text.  Copy it. Paste it into SimpleMU*.
   Or, do SimpleMU*'s File->upload text option.

Q: How do I use this program with <such-and-such> client?
A: I dunno, I use SimpleMU*.

Q: Can I have a copy of the source code?
A: No.

Q: Why not?
A: Please make sure your questions are phrased in complete sentences.  That's how FAQ's work.

Q: Whats the file name extension .mas mean?
A: I wanted the extension to be .Mascii for MushAscii at first, but I'm using a 16 bit compiler.  It won't
   allow more that a three character extension. So I made is .mas to remind me that if I ever fix it, its
   supposed to be .Mascii.

Q: How come whenever something goes wrong, the program yells 'Bizzarnage' at me?
A: Because I couldn't get that word out of my head when I was writing the program.

Q: Can I associate a file extension with this program in Win95?
A: No, not at the moment.  You can, but it won't so anything but start the program.

Q: How come this isn't a windows 95 program with little spin boxes and widgets?
A: Because that would be silly

Q: I am an avid programmer, and if you sent me the source code, I bet I could modify it and make it better!
   We would make a great team!
A: If you are such a great programmer, then make it from scratch.

Q: Oh my god! You are a girl!  I bet I could email you and tell you how great a program this is, and then
   charm you, and someday meet you in person and have sex with you!
A: Nyet.  I'll ignore any such messages.

Q: What language did you program this in?
A: English

Q: Ok, I read all this stuff and have a serious comment.  Where do I send it?
A: Log on to Elendor (elendor.sbs.nau.edu 1893), make a character, pick a culture, be active for a week.
   Then page Eiloness with your comment.

Q: C'mon, can't I just email you?
A: No, you will just end up trying to form some sort of 'bond' with me.

Q: I found a bug, you should know about it. What's your email?
A: Liar.

Q: I'm confused
A: You are ugly too.

Q: I don't get how you did such-and-such. Can you just send me that fragment of the code so I can learn?
A: That's it, no more questions.

For those of you who want to run this from batch files, or are just total nerds, here are the errorlevels:

Errorlevel 0: Everything ok
Errorlevel 1: Input file not found
Errorlevel 2: Problem writing output file