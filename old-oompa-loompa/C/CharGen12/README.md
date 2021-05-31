CoC Character Generator V1.2
Copyright (C) 2002-2003 by Markus Svensson

Call of Cthulhu is a Registered Trademark of Chaosium Inc - www.chaosium.com 

----------------------------------------------------------------------------------
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------------

INTRODUCTION
---------------------
This is a character generator for the role playing game Call of Cthulhu. It 
uses the Call of Cthulhu 5th Edition rules (more specifically, the rulebook 
that I own is the 5.2 Edition). Since this is the edition of the game I use, I 
don't know how well (or bad) it fits with the latest edition of the game rules.
I have tested the program on FreeBSD, Linux, Windows XP, and it has been tested on MacOS X by Dominic von Stoesser. 
It should work well enough on any platform with Java support though.

New in this release is the possibility to save your generated character.
It is saved as a standard ASCII text file.

New versions will be available at www.switch.to/mst and www.freshmeat.net
 
Send bugs, feature requests and general comments to: 
markus.svensson@linux.nu

Enjoy!

KNOWN LIMITATIONS
---------------------
* There is currently no support for using the special interest points.
* To make a new character, you must close the skills window, and press the
  clear button.

REQUIREMENTS
---------------------
* Java 2 compatible Java Virtual Machine
* This release (V1.2) has been compiled with Sun's JDK 1.4.1_01 for Windows.
* A graphical user interface.

USAGE
----------
* On Linux/Unix: Run the coc script file, or "java -jar                        
  CoC_Character_Generator.jar"
* On Windows, run coc.bat, or "java -jar CoC_Character_Generator.jar"

TODO
--------
* Add possibility to use special interest points.
* Restructure the skills system
* Add ability to generate HTML (or other format) character sheets
* You add request here...

HISTORY / CHANGES
---------------------------
V1.0
* Initial public release

V1.1
* Add professions and skills - Done
* Add support for allocating skill points - Done

V1.2
* Add possibility to save characters to ASCII text file - Done
* Set 1920's as the default era.
* Fixed bug that would let users specify skill values greater than 100.
