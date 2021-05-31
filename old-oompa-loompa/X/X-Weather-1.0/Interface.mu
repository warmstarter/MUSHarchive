# BEGIN FILE Interface.mu

########################################################################
#                                                                      #
#     This file creates the user interface to the weather system.      #
#  It also creates a personal object for the weather system admin      #
#  to use in maintaining the system, which allows the player to        #
#  monitor and control the system without needing to be in proximity   #
#  to the objects. See "+weather help" and "+weather control_help"     #
#  for the commands.                                                   #
#                                                                      #
########################################################################

# Global user interface

# Credits first :)

&weather_credits Weather_Interface=$+weather credits:@pemit %#=%r%r
[repeat(*,72)]%r
*[space(70)]*%r
*[space(5)]X-Weather version 1.0 patchlevel 0[space(31)]*%r
*[space(70)]*%r
*[space(5)]Coded by: Myself (lyn@matt.ksu.ksu.edu)[space(26)]*%r
*[space(16)]aka[space(51)]*%r
*[space(15)]WindSinger[space(3)]@[space(3)]TwoMoons[space(30)]*%r
*[space(15)]WeatherGuy[space(3)]@[space(3)]TwoMoons[space(30)]*%r
*[space(15)]StarBerry[space(4)]@[space(3)]Children of the Fall[space(18)]*%r
*[space(70)]*%r
*[space(5)]With many thanks to Tyleet @ TwoMoons for ideas and patience
[space(5)]*%r
*[space(70)]*%r
*[space(5)]Tested on CotF and TM[space(44)]*%r
*[space(70)]*%r
*[space(5)]This code is available via anonymous FTP from[space(20)]*%r
*[space(7)]caisr2.caisr.cwru.edu[space(42)]*%r
*[space(5)]in file[space(58)]*%r
*[space(7)]/pub/mush/mushcode/X-Weather-1.0.tar.Z[space(22)]*%r
*[space(70)]*%r
[repeat(*,72)]%r
%r
-




&weather_help Weather_Interface=$+weather help:@pemit %#=%r
+Weather commands:%r
******************%r
%r
+weather..................Show the current weather status%r
+weather help.............Show this help screen%r
+weather credits..........Show the credits for X-weather%r
%r
+predict..................Show the next weather status%r
<not enabled on all systems>%r
%r
-

&weather Weather_Interface=$+weather:@pemit %#=[setq(0,[u([v(global_data)]/
parse_weather_name,[get([loc(v(#))]/weather_obj)])])]
[switch([r(0)],NOVAL,[No weather - are you inside?],BADNAME,[Bad Weather
Object - please notify the owner of this room],
[switch([get([v(global_semph)]/working)],0,
[Weather:[iter(v(sections),%r[get([r(0)]/##_[get([r(0)]/
area_##_current)])])]],
[Weather:[iter(v(sections),%r[get([r(0)]/##_[get([r(0)]/##_backup
)])])]])])]
-

&predict Weather_Interface=$+predict:@pemit %#=[switch([v(allow_predict)],0,
[Sorry, +predict is disabled on this system],[
[setq(0,[u([v(global_data)]/
parse_weather_name,[get([loc(v(#))]/weather_obj)])])]
[switch([r(0)],NOVAL,[No weather - are you inside?],BADNAME,[Bad Weather
Object - please notify the owner of this room],
[switch([get([v(global_semph)]/working)],0,
[Prediction:[iter(v(sections),%r[get([r(0)]/##_[get([r(0)]/
area_##_next)])])]],
[Prediction:[iter(v(sections),%r[get([r(0)]/##_[get([r(0)]/##_next_backup
)])])]])])]])]
-

#####

# Remote Interface
# NOTE: Some powers of the Remote Interface should only be used by
# a dedicated weather player, due to possible interference with
# other code (+weather update)

&weather_monitor Control_Interface=$+weather monitor:@dolist get([v(
global_data)]/name_list)={@tr [get([v(global_data)]/##_number)]/report=%#}
-

# WARNING! The following commands @halt the invoker of the command.
# These commands should only be used by a dedicated weather character.
# Otherwise, this code may interfere with other running code, by
# eliminating queued commands.
# Furthermore, they should only be run when the only weather process is
# the main timer process. Running them while the weather system is updating
# may produce unpredictable results.

# +weather reset cause a system-wide reset

&weather_reset Control_Interface=$+weather reset:@halt %#;@tr [v(
global_semph)]/system_reset
-

# +weather update forces an immediate weather system update

&weather_update Control_Interface=$+weather update:@halt %#;@tr [v(
global_semph)]/start_update
-

# +weather objects will show the dbref #s of all recognized objects in
# the weather system.

&weather_objects Control_Interface=$+weather objects:@pemit %#=%r
Weather System Objects:%r
***********************%r
%r
Master_Weather_Parent: [v(master_parent)]%r
Weather_System_Semaphore: [v(global_semph)]%r
Global_Weather_Data: [v(global_data)]%r
Weather_Interface: [v(weather_interface)]%r
Control_Interface: [num(me)]%r
[iter([get([v(global_semph)]/areas_list)],%r[name(##)]: ##)]%r
-

# +weather options shows the settings for all admin options
&weather_options Control_Interface=$+weather options:@pemit %#=%r
Weather System Admin Options:%r
*****************************%r
Range: [get([v(master_parent)]/rand_range)]%r
Low: [get([v(master_parent)]/rand_low)]%r
High: [get([v(master_parent)]/rand_high)]%r
%r
Update Time: [get([v(global_semph)]/area_delay)]%r
+predict is: [switch([get([v(weather_interface)]/allow_predict)],
0,DIS)]ALLOWED%r
-


# +weather control_help shows the help for the admin controller interface

&weather_control_help Control_Interface=$+weather control_help:@pemit %#=%r
+weather control commands:%r
**************************%r
%r
+weather control_help..........This help screen%r
+weather reset.................Reset the entire weather system (WARNING!)%r
+weather update................Force an immediate system update (WARNING!)%r
+weather monitor...............Show current status of all weather objects%r
+weather objects...............Show a listing of all objects in the system%r
+weather options...............Show the status of all Admin options%r
%r
NOTE: Commands marked with (WARNING!) should only be run by a dedicated%r
weather character, due to the possibility of interference with other code,%r
and should only be run when the only weather process is the main timer.%r
%r
-

########################################################################

# END FILE Interface.mu

