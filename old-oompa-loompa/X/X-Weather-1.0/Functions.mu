# BEGIN FILE Functions.mu

########################################################################
#                                                                      #
#     This file contains numerous unrelated functions and headers/     #
#  tailers for constructing attributes. These are things that are      #
#  fed into functions such as fold(),map(), or u(), plus pieces        #
#  that are concatenated with others into attributes for later use.    #
#                                                                      #
########################################################################

# Functions

# U() function for processing object names on Global Data Object

&parse_weather_name Global_Weather_Data=switch([%0],{},[NOVAL],[switch([
member(v(name_list),[%0])],0,[BADNAME],[v([%0]_number)])])
-

# Fold() attribute to add results for master parent

&add_fold Master_Weather_Parent=add(%0,%1)
-

# Seperation to avoid ## clashes on master parent

&depend_symbol_sep Master_Weather_Parent=iter(v(depends_prev),
{:[get(##/area_[%0]_next)]})
-

# Recursive function to check differences between sections
# This allows for easy expansion into more weather sections

&check_recurse_fn Master_Weather_Parent=iter(rest(%0),{[sub(max(abs(sub(v(
area_[first(%0)]_new),v(area_##_new))),2),2)],})
-

#####

# Headers/Tailers

# Header for separation check

&check_start Master_Weather_Parent=&check_result me=[add(
-

# Tailer for separation check

&check_end Master_Weather_Parent=)];@switch/first v(check_result)=0,{@dolist 
switch([strlen(v(depends_next))],0,[v(global_semph)],[v(depends_next)])=
{@notify ##}},{@tr me/count_calcs1}
-

########################################################################

# END FILE Functions.mu

