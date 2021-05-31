# BEGIN FILE Semaphore.mu

########################################################################
#                                                                      #
#     This file contains most of the code for the inter-object         #
#  semaphore, which keeps the system synchronized, and also runs the   #
#  main timer process.                                                 #
#                                                                      #
########################################################################

# System reset first

&system_reset Weather_System_Semaphore=@drain me;@dolist v(areas_list)={
@tr ##/weather_reset};@tr me/system_start
-

# Now the main system start commands

&system_start Weather_System_Semaphore=&iter_hold me=[iter(v(areas_list),
@wait me={@@ ## reset};)][@wait me={@tr me/start_update}];@tr me/iter_hold;
@notify me
-

# Main update command

&start_update Weather_System_Semaphore=@wait [v(area_delay)]={@tr me/
start_update};@dolist v(areas_list)={@tr ##/update_system};&iter_hold me=
[iter(v(areas_report),@wait me={@@ ## reported};)][@wait me={@tr me/
start_backup}];@tr me/iter_hold;@notify me
-

# Back up attributes, so they remain available during the rotation sequence

&start_backup Weather_System_Semaphore=@dolist v(areas_list)={@tr ##/backup};
&iter_hold me=[iter(v(areas_list),@wait me={@@ ## backed up};)][@wait me={
@tr me/start_rotate}];@tr me/iter_hold;@notify me
-

# Now, rotate the system, so that the weather values advance

&start_rotate Weather_System_Semaphore=&working me=1;@dolist v(areas_list)=
{@tr ##/rotate};&iter_hold me=[iter(v(areas_list),@wait me={@@ ## rotated};)]
[@wait me={&working me=0}];@tr me/iter_hold;@notify me
-

########################################################################

# END FILE Semaphore.mu

