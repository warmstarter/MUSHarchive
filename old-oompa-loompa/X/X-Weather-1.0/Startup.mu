# BEGIN FILE Startup.mu

########################################################################
#                                                                      #
#     This file holds all of the @startup commands (these allow the    #
#  weather system to automatically reset and restart in the event of   #
#  a reboot. All weather objects are reset to straight 4s, and the     #
#  main timer process is restarted.                                    #
#                                                                      #
########################################################################

# Start the Semaphore

@startup Weather_System_Semaphore=@drain me;@tr me/system_start
-

# Set @startup on master parent, so each child auto-resets on reboot

@startup Master_Weather_Parent=@tr me/weather_reset
-

########################################################################

# END FILE Startup.mu

