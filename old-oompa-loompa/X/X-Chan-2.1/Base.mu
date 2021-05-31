# Main object creation section
#
@create X-Channels 2.0 User Commands
-
@notify X-Channels 2.0 User Commands
-
@set X-Channels 2.0 User Commands=INHERIT
-
@set X-Channels 2.0 User Commands=SAFE
-
@create X-Channels 2.0 Admin Commands
-
@set X-Channels 2.0 Admin Commands=INHERIT
-
@set X-Channels 2.0 Admin Commands=SAFE
-
@notify X-Channels 2.0 Admin Commands
-
@create X-Channels Data Storage
-
@set X-Channels Data Storage=HALTED
-
@set X-Channels Data Storage=SAFE
-
@parent X-Channels 2.0 User Commands=X-Channels Data Storage
-
@parent X-Channels 2.0 Admin Commands=X-Channels Data Storage
-
@startup X-Channels 2.0 User Commands=@drain me;@notify me
-
@startup X-Channels 2.0 Admin Commands=@drain me;@notify me
-
@desc X-Channels 2.0 User Commands=Coded by Lucifer @ Prairie Fire
-
@desc X-Channels 2.0 Admin Commands=Coded by Lucifer @ Prairie Fire
-
@desc X-Channels Data Storage=Coded by Lucifer @ Prairie Fire
-
