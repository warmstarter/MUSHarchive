Instructions for Linux ASpace Build

1) Un-gzip, Un-tar your 1.7.3p13 distribution

2) Run ./Configure -d

3) Copy the 1.7.3p13-aspace1.0.0p2 to the root of your distribution

4) patch -p1 < 1.7.3p13-aspace1.0.0p2

5) Copy all the space_*.c files to the src directory

6) Copy the space.h to the hdrs directory

7) Add #DEFINE ASPACE 1 to your options.h (after doing make update)

8) Ensure tiny_math in game/mush.cnf is set to yes (reactors get broken w/o)

9) Compile away


