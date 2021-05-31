########################################################################
#                                                                      #
#     X-Weather version 1.0 patchlevel 0                               #
#                                                                      #
#     Coded by: Myself (lyn@matt.ksu.ksu.edu)                          #
#                aka                                                   #
#               WindSinger   @   TwoMoons                              #
#               WeatherGuy   @   TwoMoons                              #
#               StarBerry    @   Children of the Fall                  #
#                                                                      #
#     With many thanks to Tyleet @ TwoMoons for ideas and patience     #
#                                                                      #
#     Tested on CotF and TM                                            #
#                                                                      #
#     This code is available via anonymous FTP from                    #
#       caisr2.caisr.cwru.edu                                          #
#     in file                                                          #
#       /pub/mush/mushcode/X-weather.1.0.p0.tar.Z                      #
#                                                                      #
########################################################################
#                                                                      #
#     NOTICE: These files are formatted for the MUSH unformatter by    #
#  Andrew Molitor, which should be available from the site where you   #
#  obtained this code, or from caisr2.caisr.cwru.edu in file:          #
#     /pub/mush/mushcode/amolitor_mush_unformatter.c                   #
#                                                                      #
########################################################################
#                                                                      #
#     FILES:                                                           #
#      README               Credits/File list/Instructions (this file) #
#      CHANGES              Changes from previous versions             #
#      TODO                 Things that remain to be done              #
#                                                                      #
#      Options.mu           Administrative options file                #
#      Create.mu            Create and set up basic objects            #
#      Interface.mu         Code for Global and Control Interfaces     #
#      Master.mu            Code for Master Weather Parent             #
#      Semaphore.mu         Code for Global Weather Semaphore          #
#      Functions.mu         Various functions, plus headers/tailers    #
#      Startup.mu           @startup code for entire system            #
#      Setup.mu             Defaults for hand-set values               #
#                                                                      #
########################################################################
#                                                                      #
#     This system is a complete, functioning weather system written    #
#  in TinyMUSH 2.0 form. It does, however, require some thought and    #
#  a small amount of human setup. Administrative options can be found  #
#  in the file Options.mu, including update frequency and probability  #
#  limits. You will also need to design the 'flow control' of your     #
#  system. See file DESIGN for instructions on how to do this. The     #
#  file INSTRUCTIONS contains a basic checklist for installation.      #
#  There is an internal +help system for both the Global and Control   #
#  weather interfaces. See +help weather and +help weather_control.    #
#                                                                      #
########################################################################

