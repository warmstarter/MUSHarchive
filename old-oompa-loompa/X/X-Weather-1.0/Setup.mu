# BEGIN FILE Setup.mu

########################################################################
#                                                                      #
#     This file holds all attributes which should be defined as the    #
#  system is being set up. ALL of these should be hand-set, to the     #
#  appropriate values, but this can only be done inside the game.      #
#  These values are only reminders to fill in the real values.         #
#                                                                      #
########################################################################

# The list of all weather objects must wait until they have been created

&areas_list Weather_System_Semaphore=<List of all weather objects>
-

# The end objects of the chains must wait until chains have been made

&areas_report Weather_System_Semaphore=<List of areas that end chains>
-

# The name list for the Global Data Object must have all reference names

&name_list Global_Weather_Data=<List of Weather Object reference names>
-

########################################################################

# END FILE Setup.mu

