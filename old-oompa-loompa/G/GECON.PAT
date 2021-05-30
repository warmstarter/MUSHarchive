*** funlocal.c.orig	Thu Nov 30 17:21:12 2000
--- funlocal.c	Thu Nov 30 17:33:30 2000
***************
*** 30,35 ****
--- 30,58 ----
   * Example included :)
   */
  
+  
+ FUNCTION(local_fun_trigger)
+ {
+   if(!command_check_byname(executor, "@trigger"))
+        safe_str("#-1 PERMISSION DENIED", buff, bp);
+   else do_trigger(executor, args[0], args);
+ }
+ 
+ FUNCTION(local_fun_wait)
+ {
+   if(!command_check_byname(executor, "@wait"))
+        safe_str("#-1 PERMISSION DENIED", buff, bp);
+   else do_wait(executor, executor, args[0], args[1]);
+ }
+ 
+ FUNCTION(local_fun_force)
+ {
+   if(!command_check_byname(executor, "@force"))
+         safe_str("#-1 PERMISSION DENIED", buff, bp);
+   else do_force(executor, args[0], args[1]);
+ }
+ 
+ 
  #ifdef EXAMPLE
  FUNCTION(local_fun_silly)
  {
***************
*** 41,46 ****
--- 64,72 ----
  void
  local_functions()
  {
+   function_add("TRIGGER", local_fun_trigger, 1, 11, FN_REG);
+   function_add("WAIT", local_fun_wait, 2, 2, FN_REG);
+   function_add("FORCE", local_fun_force, 2, 2, FN_REG);
  #ifdef EXAMPLE
    function_add("SILLY", local_fun_silly, 1, 1, FN_REG);
  #endif
