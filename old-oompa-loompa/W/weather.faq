Keran's MUX/MUSH Weather and Time Code Package
Frequently Asked Questions
Current MUX, PennMUSH, and TinyMUSH-2.2 Versions: 4.0 
TinyMUSH 3.0: MUX 4.0 and patch.


1. What servers does the weather and time code run on?

TinyMUX 1.4 and 1.6; it probably runs on earlier MUX versions,
but the current release hasn't been tested on them.

The code will execute on TinyMUX 1.5, which is a beta release,
but there are memory leaks in it, which this code appears
to invoke; it should not be run on 1.5 without very frequent
shutdowns, as otherwise the memory leaks cause your process
size to grow without bounds and eventually can crash your
host.

4.0 has been ported to PennMUSH 1.7.2 and TinyMUSH 2.2.2.
There is now a patch allowing the MUX version to work on
TinyMUSH 3.0.

TinyMUSH 3.0b20, the latest release as of this writing,
also has memory leaks. If you're going to install the
Weather and Time Code on it, you should @restart your
server periodically. Once a day is good.

2. Will you port it to ...?

I'm not going to port it to any non-MUSH-family servers, but
you can if you like. 


3. What are the features of the weather and time code?

The 3.0 features:

 -- Time functions for determining the year, month, season, time of day,
  tide, and moon phase; 
 -- A weathermaker that determines a different weather type every day; 
 -- Day and night length that vary by season and can be adjusted for
  approximate latitude;
 -- A weather description bank with descriptions that vary by time of day
  and season; 
 -- An outdoor parent room that makes it easy to write descriptions that
  vary by time of day, season, weather, and the visibility of the
  moon;
 -- Configurable time compression ratios, in two different modes. You can
  accelerate all time with clock compression, or skip days with
  calendar compression. You can run at 4:1, 3:1, 2:1, 3:2, or 1:1 by
  changing a few settings; 
 -- Simple adjustable weather modelling; 
 -- An optional quasi-Gregorian calendar in addition to the native
  calendar; 
 -- Clock code that can send out messages to specified receivers at given
  times, such as sunrise, moonset, other time of day transitions, or
  at specific hours.
 -- An auxiliary bank of descriptions varying by season, time of day, and
  weather, geared to surroundings with a lot of leafy trees, to
  illustrate how to make auxiliary desc banks for different
  environments; 
 -- Regional weather, and a sample slave weather station.

4.0 also has:

 -- A collection of outdoor parent rooms that make it easy to write
  descriptions that vary by time of day, season, weather, and the
  visibility of the moon, in different styles;
 -- Indoor parent rooms for writing descriptions that vary with lighting
  conditions and the visual capability of the viewer;
 -- A parent room hierarchy that means you can write your own parent
  rooms and easily have them recognized as the correct type of room;
 -- An option for taking the date from the system clock in 1:1 time; 
Clock code that can send out messages to specified receivers every
  hour, every half-hour, or every quarter hour.
 -- A reasonably realistic moon; 
 -- Diurnal or semi-diurnal tides synchronized with the moon;
 -- A Storm Maker that can be used to do global, area, and room type emits
  to allow the production of storms, earthquakes, blackouts, etc.
 -- A test room for ensuring that the whole thing works.
 -- Even more confusing +help. ("Impossible!" cry the skeptics. "Your
  code is easier to read than your +help!")
 -- OK, seriously, reorganized +help. I hope it's clearer.


4. What's planned for the next version?

I'm not planning a next version. At this time, I don't
know whether someone else will take over maintaining the
code. If they do, it's up to them.

5. What environments can it model?

Almost any environment with earthlike weather you feel like writing
description banks for. The supplied description banks are for a moist
northern hemisphere mid-latitude temperate climate; the short
descriptions on the Weathermaker, and the general weather descriptions
on the Weather Long Description bank, describe atmospheric conditions
in such an environment. You can rewrite them so they suit warmer or
colder climates. There is also one auxiliary description bank
supplied, the Tree Weather Description Bank, which gives descriptions
of a temperate environment with a lot of deciduous trees; the
descriptions vary by day, night, season, and weathertype. It is
simple, although laborious, to write description banks for cities,
deserts, grassland, seacoasts, etc.

You can also change the frequency with which different weathertypes
are selected by the Weathermaker at various seasons. For example, the
default setting produces a lot of fog in summer because it's for an
island. It would be comparatively simple to reduce the amount of
summer fog but increase the frequency of snow in winter for an upland
setting; this is just a question of varying the number of times a
given weathertype appears in the selection lists on the Weathermaker.

You will have to change the order of the Gregorian months on the Time
Functions object if you're using the quasi-Gregorian calendar for a
setting in the southern hemisphere.


6. Can I use the weather code without the timing code?

Yes, if you supply other timing code; it is strongly not recommended
that you attempt this unless you're a competent coder; the timing
is the guts of the system. In order to use foreign timing code and
preserve most of the functionality of this weather system, the timing
code must capable of the following:

 -- It must, at the very least, distinguish between day and night;
preferably, it distinguishes between day, night, dawn, and dusk.

 -- It must distinguish the seasons spring, summer, autumn, and winter.

 -- In order for the outdoor parent rooms to work as it does in this
system, there must be a function that tells whether the moon is up
or not.

 -- There must be a clock capable of triggering the Weathermaker at dawn.


7. Can I add moons?

Yes. There are instructions for doing it in the configuration
section of the +help.


8. Can I set up weather that varies from region to region?

Yes. You can even track the probable course of storms from region to
region. A sample slave weather station is provided, along with
documentation for how to set up different weather regions. It isn't
recommended that you attempt this unless you are reasonably competent
with code; being able to read and understand some of the code is a
necessity for the setup.


9. Can I set up different time zones?

Probably, although it would take separate copies of the code suite
for each time zone, with a different value for time-begins set on
each copy of the Time Functions object. It hasn't been tested.


10. Can I leave any of these objects out?

Yes. If you don't want regional weather, you don't need the Slave
Weather Station. If you don't have any use for descriptions of an
environment with a lot of broadleafed trees, leave out the Tree
Weather Descriptions Bank. If you don't want to use the global time
of day emits, you can omit the Emits bank, and if you don't want to
use the Storm Maker or the parent rooms, you can leave them out. If
you don't want a moon, you can leave the Moon and Tide object out,
and if you don't want to test the installation, you can leave the
Test Room out. You can also get rid of the test room after you do
test it.


13. What time compression ratios can I use it at?

In calendar compression mode, 4:1, 3:1, 2:1, and 1:1. In clock
compression mode, it's been tested at 4:1, 3:1, 2:1, 1:1, and 3:2. It
is expected to run at any clock compression ratio involving small
whole numbers by which 3600 can be evenly divided, e.g, it should run
at 5:2 and 1:2, but it has not been tested at all such ratios.


14. Where do I get the code?

As of this writing, it's still up at
ftp://telmaron.telmaron.com/pub/keranset. I don't know where
its permanent home will be yet.

15. What do I do if I find a bug?

If this code has a new maintainer, email them and tell them
what went wrong, what version of the code you have installed,
and what platform you're running on. If it doesn't, you may
need to get a coder to fix it, or fix it yourself.

16. What's the best version to install on TinyMUSH 3.0?

Install the Mux version 4.0; it's more capable than the
TinyMUSH 2.2 version. Then apply the TM3 patch.


Keran
