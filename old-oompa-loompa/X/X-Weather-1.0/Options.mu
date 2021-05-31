# BEGIN FILE Options.mu

########################################################################
#                                                                      #
#    The following section contains all administrative settings        #
#    Unless you are very familiar with MUSH coding, it is advised      #
#    to avoid changing anything except these values                    #
#                                                                      #
########################################################################

# The following numbers are used to determine the probability of a
# change in weather. The defaults make it 20% worse, 20% better, 60%
# no change.

# Total range of random numbers (0..rand_range-1)

&rand_range Master_Weather_Parent=5
-

# Numbers less than rand_low worsen weather

&rand_low Master_Weather_Parent=1
-

# Numbers greater than rand_high improve the weather

&rand_high Master_Weather_Parent=3
-

#####

# This number determines how often the weather system updates.
# The number is in seconds. 900 seconds is 15 minutes.

&area_delay Weather_System_Semaphore=900
-

#####

# Allow_predict flags whether players should be allowed to access the
# +predict function. Unless there is a good reason to have it, it is
# generally pointless, so it is disabled by default.

&allow_predict Weather_Interface=0
-

########################################################################

# END FILE Options.mu

