# BEGIN FILE Master.mu

########################################################################
#                                                                      #
#     This file contains the majority of the code for the Master       #
#  Weather Parent. This includes code to reset, update, and rotate     #
#  the system. (Timing for these is initiated by the Semaphore)        #
#                                                                      #
########################################################################

# Start out with the master reset code

&weather_reset Master_Weather_Parent=@drain me;&counter me=0;@tr me/
area_reset;@tr me/reset_wait
-

# Now the area reset code

&area_reset Master_Weather_Parent=@dolist v(sections)={@tr me/
area_section_reset=##}
-
&area_section_reset Master_Weather_Parent=@dolist v(area_parts)={@tr me/
area_part_reset=[%0],##}
-
&area_part_reset Master_Weather_Parent=&area_[%0]_[%1] me=4;&counter me=
[add(v(counter),1)]
-

# Timing loop - wait for reset to complete

&reset_wait Master_Weather_Parent=&counter_target me=[mul(words(v(sections)),
words(v(area_parts)))];@tr me/reset_wait0
-
&reset_wait0 Master_Weather_Parent=@wait 1={@tr me/reset_wait[eq(v(counter),
v(counter_target))]}
-
&reset_wait1 Master_Weather_Parent=@notify [v(global_semph)]
-

#####

# Update code

# Main update function

&update_system Master_Weather_Parent=&iter_hold me=[iter(v(depends_prev),
@wait me={@@ ## ready};)][@wait me={@tr me/set_avg}];@tr me/iter_hold;
@notify me
-

# Bring in the values of the other objects appropriate to influence

&set_avg Master_Weather_Parent=&counter me=0;@dolist v(sections)={
&area_##_calc me=0;&##_add me=0;&##_add me=[fold(add_fold,[u(
depend_symbol_sep,##)],[v(area_##_next)],:)];@tr me/wait_added0=##;
@tr me/wait_calc0=##};@tr me/count_calcs0
-

# Wait loops to delay until numbers are available

&wait_added0 Master_Weather_Parent=@wait 1={@tr me/wait_added[neq(v(
[%0]_add),0)]=[%0]}
-
&wait_added1 Master_Weather_Parent=&area_[%0]_calc me=[round(fdiv(v(
[%0]_add),add(words(v(depends_prev)),1)),0)]
-
&wait_calc0 Master_Weather_Parent=@wait 1={@tr me/wait_calc[neq(v(
area_[%0]_calc),0)]=[%0]}
-
&wait_calc1 Master_Weather_Parent=&counter me=[add(v(counter),1)]
-

# Wait until all sections are calculated, then do random number selection

&count_calcs0 Master_Weather_Parent=@wait 1={@tr me/count_calcs[eq(v(
counter),words(v(sections)))]}
-
&count_calcs1 Master_Weather_Parent=&counter me=0;@dolist v(sections)={
&##_rand_hold me=[rand(v(rand_range))];@tr me/adjust_weather[lt(v(##
_rand_hold),v(rand_low))][gt(v(##_rand_hold),v(rand_high))]=##};
&check_holder me=;@tr me/adjust_wait0
-

# Adjust weather storage values according to random selection

&adjust_weather00 Master_Weather_Parent=&area_[%0]_new me=[v(area_[%0]_calc)];
&counter me=[add(v(counter),1)]
-
&adjust_weather01 Master_Weather_Parent=&area_[%0]_new me=[min(add(v(
area_[%0]_calc),1),7)];&counter me=[add(v(counter),1)]
-
&adjust_weather10 Master_Weather_Parent=&area_[%0]_new me=[max(sub(v(
area_[%0]_calc),1),1)];&counter me=[add(v(counter),1)]
-

# Wait until all adjustments are made, then set the checking routine

&adjust_wait0 Master_Weather_Parent=@wait 1={@tr me/adjust_wait[eq(v(
counter),words(v(sections)))]=[v(sections)]}
-
&adjust_wait1 Master_Weather_Parent=&check_holder me=[v(check_holder)][u(
check_recurse_fn,[%0])];@tr me/adjust_wait[add(lte(words([%0]),2),1)]=[
rest([%0])]
-
&adjust_wait2 Master_Weather_Parent=&check_holder me=[v(check_start)][v(
check_holder)][v(check_end)];@tr me/check_holder
-

#####

# Backup code - backup all weather attibutes before rotating, allows
# system access while running rotation.

# Main backup function

&backup Master_Weather_Parent=&counter me=0;@dolist v(sections)={
&##_backup me=[v(area_##_current)];&##_next_backup me=[v(area_##_next)];
&counter me=[add(v(counter),1)]};@tr me/backup_wait0
-

# Wait until all backups are complete

&backup_wait0 Master_Weather_Parent=@tr me/backup_wait[eq(v(counter),words(
v(sections)))]
-
&backup_wait1 Master_Weather_Parent=@notify [v(global_semph)]
-

#####

# Rotation code (advance weather values)

# Main roatation sequence

&rotate Master_Weather_Parent=&counter me=0;&counter_target me=[words(v(
sections))];@tr me/rotate_area;@tr me/rotate_wait0
-

# Area rotation code

&rotate_area Master_Weather_Parent=@dolist v(sections)={@tr me/
rotate_area_section0=##,[v(area_parts)]}
-
&rotate_area_section0 Master_Weather_Parent=&area_[%0]_[first([%1])] me=
[v(area_[%0]_[first(rest([%1]))])];@tr me/rotate_area_section[lte(words([
%1]),2)]=[%0],[rest([%1])]
-
&rotate_area_section1 Master_Weather_Parent=&counter me=[add(v(counter),1)]
-

# Wait until all rotations are complete

&rotate_wait0 Master_Weather_Parent=@wait 1={@tr me/rotate_wait[eq(v(
counter),v(counter_target))]}
-
&rotate_wait1 Master_Weather_Parent=@notify [v(global_semph)]
-

#####

# Stick the monitor response here, just because

&report Master_Weather_Parent=@pemit %0=[name(me)]([num(me)]):
[iter(v(sections),%r##: [v(area_##_past)]/[v(area_##_current)]/
[v(area_##_next)])]%r
-

########################################################################

# END FILE Master.mu

