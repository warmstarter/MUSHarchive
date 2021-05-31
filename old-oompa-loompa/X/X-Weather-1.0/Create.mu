# BEGIN FILE Create.mu

########################################################################
#                                                                      #
#     This section will create all the necessary global objects, and   #
#     set their flags and initial values.                              #
#                                                                      #
########################################################################

# Create the main weather parent

@create Master_Weather_Parent
-
@set Master_Weather_Parent=VISUAL
-
@set Master_Weather_Parent=HALTED
-

# Create inter-object semaphore

@create Weather_System_Semaphore
-
@set Weather_System_Semaphore=HALTED
-

# Create global data reference object

@create Global_Weather_Data
-

# Create user interface

@create Weather_Interface
-
@set Weather_Interface=HALTED
-

# Create remote control interface
# SEE later INHERIT flag (this file)

@create Control_Interface
-
@set Control_Interface=HALTED
-

# Create a temporary working object

@create Work_Object
-

#####

# Set up attributes to capture/recall object numbers

&get_semaphore Work_Object=&store_semaphore me=[num(
Weather_System_Semaphore)]
-
&get_data Work_Object=&store_data me=[num(Global_Weather_Data)]
-
&get_master_parent Work_Object=&store_master_parent me=[num(
Master_Weather_Parent)]
-
&get_weather_interface Work_Object=&store_weather_interface me=[num(
Weather_Interface)]
-

&set_semaphore Work_Object=&global_semph %0=[v(store_semaphore)]
-
&set_data Work_Object=&global_data %0=[v(store_data)]
-
&set_master_parent Work_Object=&master_parent %0=[v(store_master_parent)]
-
&set_weather_interface Work_Object=&weather_interface %0=[v(
store_weather_interface)]
-

# Capture object numbers of global reference objects

@tr Work_Object/get_semaphore
-
@tr Work_Object/get_data
-
@tr Work_Object/get_master_parent
-
@tr Work_Object/get_weather_interface
-

# Set global object numbers in attribute for objects that need them

@tr Work_Object/set_semaphore=Master_Weather_Parent
-
@tr Work_Object/set_semaphore=Weather_Interface
-
@tr Work_Object/set_data=Weather_Interface
-
@tr Work_Object/set_semaphore=Control_Interface
-
@tr Work_Object/set_data=Control_Interface
-
@tr Work_Object/set_master_parent=Control_Interface
-
@tr Work_Object/set_weather_interface=Control_Interface
-

#####

# Set up default values for basic attributes

# Sections holds the names of the weather sections

&sections Master_Weather_Parent=wind sky temp
-
&sections Weather_Interface=wind sky temp
-

# Area_parts holds the names of the different attributes for storing values.

&area_parts Master_Weather_Parent=past current next new calc
-

# Working flags a weather system update in progress (use backup attributes)

&working Weather_System_Semaphore=0
-

# This value must be visual, becuase it must be referenced by others

@set Weather_System_Semaphore/working=VISUAL
-

# Depends_next and Depends_prev are set to reminder values on the master
# parent, but are set NO_INHERIT to avoid unintentional inheritance of
# these values (if a child has an empty list)

# Depends_prev holds a list of objects whose states affect the current object

&depends_prev Master_Weather_Parent=<list of objects that influence this one>
-

# Depends_next holds a list of objects whose states are affected by this one

&depends_next Master_Weather_Parent=<list of objects that are influenced by
this one>
-

# Set both of these values NO_INHERIT to avoid unintentional inheritance

@set Master_Weather_Parent/depends_prev=NO_INHERIT
-
@set Master_Weather_Parent/depends_next=NO_INHERIT
-

#####

# Needed for some commands. NOTE POSSIBLE SECURITY RISKS
# Placed here to avoid problems with Work_Object triggers.

@set Control_Interface=INHERIT
-

########################################################################

# END FILE Create
