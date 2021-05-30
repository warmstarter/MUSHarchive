#####################################################################
#                 		BUY COMMAND                         #
#                                                                   #
# This code is to enable players to buy and sell to one another     #
# to the NPC's and to the economy at large via the market.          #
#                                                                   #
# Where #99 is the Economic Command Object                          #
# Where #24 Firan Demand Market Object                              #
# Where #25 The Firan Marketplace                                   #
# Where #26 Firan Supply Market Object                              #
# Where #27 Firan Cost Market Object                                #
#                                                                   #
#####################################################################

&cmd-for-sale #26=$for sale:

  think setq(0, laccounts(#26));
  think setq(0, filter(#25/filter-supplies, %q0));
  @pemit %#=%ch%cgFor Sale Today:%r%r%cn 
            [columns(iter(%q0, %b[clip(##)], %b, |), 25, |)]

-

#####################################################################
#  BUY <AMOUNT> <COMMODITY> FROM <SELLER>                           #
####################################################################
#
# %q0 = amount to buy
# %q1 = commodity type
# %q2 = dbref of seller
# %q3 = worth/going price for commodity

&cmd-buy #99=$buy * * from *:

 think setq(0, trim(%0));
 think setq(1, trim(%1));
 think setq(1, switch(iscurrency(%1), 1, %q1, %1s));
 think setq(1, switch(iscurrency(%q1), 1, %q1, %1es));
 think setq(1, switch(iscurrency(%q1), 1, %q1, 
                      after(grab(get(#30/exceptions), %1=*, |),=)));
 think setq(2, locate(%#, %2, ainX));
 think setq(3, price(%q1));
 think setq(4, mul(%q3, %q0));

# Error check:
#  Is the syntax for the market?
#  is the amount specified a number?
#  is the commodity an actual valid currency
#  is the seller a valid seller?
#  does seller even have that many to sell?
#  is the buyer a player?

# To avoid the blowing of the buffer limit in excess(), we're going 
# to put this in a separate switch.

 @switch [or(strmatch(%2, market*), strmatch(%1, market*))]=
    1, {@@ do nothing, other syntax will pick it up @@},
    0, { @fpose %#=tries to buy %q0 %q1 from [name(%q2)].;

         think setq(9, [and(isnum(%q0), iswhole(%q0))]
                      [iscurrency(%q1)]
                      [and(isdbref(%q2), hastag(%q2, npc))]
                      [gte(excess(%q2, %q1), %q0)]
                      [gte(getaccount(%#, stenis), %q4)]
                      [and(hastype(%#, player), not(hastag(%#, npc)))]);

#         @pemit *steph=%q9;

         @switch/first %q9=


           0?????, {@pemit %#=You failed to specify a valid amount to buy.;
                    @fsay %q2=You have to tell me how much you want to buy, 
                         [name(%#)] },
  
           ?0????, {@pemit %#=You failed to specify a valid commodity to buy.;
                    @fsay %q2=What is it that you want to buy, [name(%#)]?;
                    @fpose %q2=looks puzzled. },

           ??0???, {@pemit %#=You failed to specify a valid seller.  You can 
                          only use the 'buy' command to purchase things from 
                          NPC's.  Otherwise, you should roleplay your 
                          purchases. },
 
           ???0??, {@fsay %q2=I don't have
                          [switch(1, lt(excess(%q2, %q1), 1), any, %q0 )] 
                          %q1 to sell.;
                    @switch [hastag(paccount(%q1), poison)]=
                           1, {@tr me/trig-buy-poison=%#, %q0, %q1},
                              {@@ do nothing since they're not buying poison}},
  
           ????0?, {@fsay %q2=I'm selling %q1 for %q3 a piece.  You sure you 
                      have enough money?;
                    @switch [hastag(paccount(%q1), poison)]=
                           1, {@tr me/trig-buy-poison=%#, %q0, %q1},
                              {@@ do nothing since they're not buying poison}},
           111110, {@@ do code for npc buying @@;
                    @money/transfer %q4 from %# to %q2;
                    think ac_transfer(%q0, %q1, %q2, %#) },

           111111, {@@ do code for player buying from npcs @@;
                    @money/transfer %q4 from %# to %q2;
                    @extract %q0/%q1=%q2/%q1=[loc(%#)];
                    @fpose %#=hands [name(%q2)] %q4 stenis.;
                    @fpose %q2=puts the %q1 where [name(%#)] can take them 
                          and thanks [obj(%#)] for [poss(%#)] business.;

#                    Added so that we can keep track of who owns what
                     &owner },

   {@@ default code to run if there's been an error @@;
    @pemit %#=Error: Admin have been alerted.;
    @qmail steph/buy command error=Unexpected Condition on cmd-buy 
           [num(me)] when [name(%#)] %# typed: 
           buy %0 of %1 from %2: %q9}}

-
&u-exceptions #26=[get_eval(#30/exceptions)]
-

#####################################################################
#  BUY <AMOUNT> <COMMODITY> FROM <SELLER>                           #
#  BUY <AMOUNT> <COMMODITY> FROM MARKET                             #
#####################################################################
#
# %q0 = amount to buy
# %q1 = commodity type
# %q2 = commodity type narrowed down for plurals
# %q3 = the market object
#====================================================================


&cmd-buy-market #26=$buy * from market*:

  think setq(0, switch(isnum(trim(first(%0))), 1, trim(first(%0)), 1));
  think setq(1, switch(isnum(trim(first(%0))), 1, trim(rest(%0)), trim(%0)));
  think setq(2, switch(iscurrency(%q1), 1, %q1, %q1s));
  think setq(2, switch(iscurrency(%q2), 1, %q2, %q1es));
  think setq(2, switch(iscurrency(%q2), 1, %q2, 
                      after(grab(get(#30/exceptions), %q1=*, |),=)));
  think setq(2, ifelse(iscurrency(%q2), %q2, %q1));
  think setq(3, #26);

#---------------------------------------------------------------------
# Error check:
#  is the amount specified a number?
#  is the commodity an actual valid currency
#  is the market allowed to sell that currency?
#  is it daytime or night time?
#  does the player have enough energy to make the attempt?
#  does the player have enough social points to make the attempt?
#---------------------------------------------------------------------


 think setq(9, [and(isnum(%q0), iswhole(%q0))]
               [iscurrency(%q2)]
               [not(hastag(paccount(%q2), nobuy))]
               [gte(add(getaccount(%#, energy), 
                        getaccount(%#, energy reserves)), v(cost-buy))]
               [or(strmatch(get(#63/time-of-day), day),
                   gte(getaccount(%#, social points), v(sp-cost-buy)))]
               [strmatch(get(#63/time-of-day), night)]);

  @switch/first %q9=
     
    0?????, {@pemit %#=You must specify a whole number as the amount 
              of the commodity you wish to buy.},

    ?0????, {@fo %#=@instock %q2;@pemit %#=%q2 is/are not a known commodity.},

    ??0???, {@pemit %#=Sorry, but merchants won't sell that to you.},

    ???0??, {@pemit %#=You're too tired to go shopping.  You don't have 
                   enough energy.},

    ????0?, {@pemit %#=You aren't owed enough favors by the merchants -- 
                   too few social points.},

    ?????0, {think take_energy(%#, v(cost-buy), both, r);
          @pemit %#=%ch%cg
                 You use up [v(cost-buy)] energy points in the attempt.;
          @fpose %#=negotiates with a merchant for %q2.;
          @fo %#=@instock %q2;
          @tr me/trig-buy-day = %q0, %q2, %q3, %#}, 

    ?????1, {think take_energy(%#, v(cost-buy), both, r);
             @pemit %#=%ch%cg
                 You use up [v(cost-buy)] energy points in the attempt.;
          think ac_adjust(%#, social points, -[v(sp-cost-buy)]);
          @pemit %#=%ch%cc
                 You use up [v(sp-cost-buy)] social points in the attempt.; 
          @fpose %#=tries to find a merchant to sell [obj(%#)] some %q2.;
          @fo %#=@instock %q2;
          @tr me/trig-buy-night = %q0, %q2, %q3, %#},

    {@qmail steph/buy from market=There's an unexpected condition 
        produced by cmd-buy-market on #26: %q9}

-
&sp-cost-buy #26=5
-

&cost-buy #26=1
-


#=================== TRIG-BUY-NIGHT ==========================#
#                                                             #
# %0 = amount to buy                                          #
# %1 = commodity type                                         #
# %2 = #26                                                    #
# %3 = dbref of the player                                    #
#                                                             #
#=============================================================#

&trig-buy-night #26=

  think setq(0, u(u-success-streetwise, %3));
  
  @switch %q0=
     0, {@fpose %3=cannot find any merchant willing to sell 
                [obj(%3)] any %1 at this hour.},

    <0, {@fpose %3=finds a merchant who is closing up, but the 
                merchant is irritated to be approached at this 
                hour and makes a rude gesture to [name(%3)].},

    >0, {@fpose %3=manages to find a merchant willing to do 
                business with [obj(%3)] at this late hour and they 
                dicker over the price.;
         @tr me/trig-buy-day = %0, %1, %2, %3}
-




#=================== TRIG-BUY-DAY ============================#
#                                                             #
# %0 = amount to buy                                          #
# %1 = commodity type                                         #
# %2 = #26                                                    #
# %3 = dbref of the player                                    #
#                                                             #
#=============================================================#

&trig-buy-day #26=

#-------------------------------------------------------------#
# Figure out what would be charged for the item given         #
# the player's negotiation skill.                             #
#                                                             #
# %q0 = the amount to buy                                     #
# %q1 = number of successes at negotiation                    #
# %q2 = price the merchant will ask for                       #
# %q3 = five percent sales tax                                #
#-------------------------------------------------------------#

  think setq(0, %0);
  think setq(1, u(u-success-negotiation, %3));


#---------------------------------------------------------------
# The discounted price that the player might receive per item
#---------------------------------------------------------------
  think setq(2, ulocal(ul-discount, %1, %q1));

#---------------------------------------------------------------
# That price times the number of items to be bought
#---------------------------------------------------------------
  think setq(2, round(mul(%q2, %0), 0));

#---------------------------------------------------------------
# If the price is zero, make it one steni by default
#---------------------------------------------------------------
  think setq(2, ifelse(lte(%q2, 0), 1, %q2));

#---------------------------------------------------------------
# Add five percent sales tax
#---------------------------------------------------------------
  think setq(3, round(mul(%q2, .05), 1));
  think setq(2, add(%q2, %q3));

#-------------------------------------------------------------#
# Add poison rumor code:
#-------------------------------------------------------------#
 
  @switch [hastag(paccount(%1), poison)]=
     1, {@tr me/trig-buy-poison=%0, %1, %3};

#-------------------------------------------------------------#
# Error checking:                                             #
#                                                             #
# See if seller has enough of the commodity to sell           #
# See if the buyer has enough money                           #
#                                                             #
#-------------------------------------------------------------#

  think setq(9, [gte(getaccount(#26, %1), %0)]
                [gte(getaccount(%3, stenis), %q2)]);


  @select %q9=
     0?, {@pemit %3=You can't find a merchant that has that 
                 many %1 to sell you.},
     ?0, {@pemit %3=It'll cost you %q2 steni(s) to buy that many 
                 %1, and you don't have enough money.},
     11, {@@ do code for player buying @@;
          think setq(4, %2);
          think setq(5, %3);
          think setq(6, %1);
          @program %3=me/prog-agree-sale:%ch
                   It'll cost you %q2 steni(s) total for %0 %1. 
                   Do you want to buy?%cn}


-
# %0 = how much to buy
# %1 = currency of poison
# %2 = %#

&trig-buy-poison #26=
  @witness/75% [name(%2)]/was seen buying %0 %1 at the market [ictime()];
  @gossip/add [name(%2)] was seen buying %0 %1 at the market 
              [extract(ictime(), 5, 9)]
-


&PROG-AGREE-SALE #26=
 @switch %0=
   yes, {@money/transfer %q2 from %q5 to %q4;
         @extract %q0/%q6=%q4/%q6=[loc(%q5)];
         @fpose %q5=hands a merchant %q2 stenis.;
         @remit [loc(%q5)]=A merchant puts the %q6 where [name(%q5)] 
                can take them and thanks [obj(%q5)] for 
                [poss(%q5)] business.;
         @tax/federal sales/#26=%q2 },
  no, {@fpose %q5=tells the merchant to forget it.},
 {@program %#=me/prog-agree-sale:%chDo you want to buy? Yes or no?%cn}

-

@set #26=wizard

-

&u-success-negotiation #26=
  
  switch(gt(skill(%0, negotiation), 0), 
     1, successes(add(skill(%0, negotiation), attribute(%0, allure)), 6),
     0, 0)
 
-
# ulocal(ul-discount, <commodity>, <number of successes in negotiation>)
#
# %q0 = the absolute number of successes ( -3, would be 3) (will be added 
#       or subtracted from the price later)
# %q1 = price of the commodity
# %q2 = 10 % of the price
# %q3 = 10 % discount times the number of successes

&ul-discount #26=

 [setq(0, trim(%1, l, -))]
 [setq(1, price(%0))]
 [setq(2, mul(%q1, .1))]
 [setq(3, mul(%q2, %q0))]
 [setq(4, switch(1, gt(%1, 0), sub(%q1, %q3), 
                    lt(%1, 0), add(%q1, %q3), 
                    %q1))]
%q4
 
-
# u(u-success-streetwise, <player>)
# 
# Returns the number of successes on a streetwise roll.

&u-success-streetwise #26=
 successes(skill(%0, streetwise), 6)

-

#===================================================================#
#              COMMAND: BUY <OBJECT>                                #
#                                                                   #
# This command is to be used by the NPC pawners who have objects    #
# to sell.                                                          #
#                                                                   #
# Where the Generic NPC Puppet is #3345                             #
#===================================================================#
@power #3345=!immutable
-
&cmd-buy-object #3345=$buy *:
  think setq(0, locate(me, %0, i));
  think setq(1, iscurrency(%0));
  think setq(9, [not(or(strmatch(%0, * from market*),
                        strmatch(%0, * from *)))]
                [and(isdbref(%q0), hastag(%q0, forsale))]
                [and(%q1, gte(getaccountd(me, %0), 1))]);

  @switch/first %q9=
      0??, {@@ do nothing, other syntaxes will pick up as appropriate @@},
      ?1?, {@tr me/trig-buy-object=%#, %q0, [get(%q0/currency)] },
      ??1, {@tr me/trig-buy-account=%#, %0},
      ???, {@fo me=pose looks at [name(%#)] and says, "I don't have 
                anything that even sounds like %0 to sell you."}
-
# One day will change this so that it does real work, but for now
# they can buy from the market.

&trig-buy-account #3345=
  @fo me=pose says to [name(%0)], "I'm not sellin anything like that 
      today, sorry.  You might try to get it from the market!"
-


# %0 = the buyer
# %1 = the object to sell
# %2 = currency type of the item
#
# %q0 = number of successes player got with negotiation skill
# %q1 = the quality of the item
# %q2 = the condition of the item
# %q3 = the appraised value of the item
# %q4 = take the number of successes with negotiation and multiply it
#     by 10 percent whether it was failures or successes, then, if it was
#     a failure, add that amount to the price.  If it was a success,
#     subtract from the price.
# %q5 = the price the seller will agree to sell at.

&trig-buy-object #3345=
  think setq(0, u(#26/u-success-negotiation, %0));
  think setq(1, ulocal(#99/ul-quality-appraise, %1));
  think setq(2, ulocal(#99/ul-condition-appraise, %1));
  think setq(3, ulocal(#99/ul-value-appraise, %1, %q1, %q2));
  think setq(4, mul(abs(%q0), .1));
  think setq(4, ifelse(gt(%q0, 0), sub(%q3, %q4), add(%q3, %q4)));
  think setq(5, round(mul(%q4, u(u-profit)),0));

# Adding in sales tax: from #97
 think setq(8, round(mul(get(#97/federal-sales-tax), %q5), 1));
  think setq(5, add(%q5, %q8));

  think setq(6, %0);
  think setq(7, %1);

  @fpose me=motions to [name(%0)]. "I'll sell the [goodname(%1)] 
         to you for %q5 steni(s).";
  @switch [gte(getaccount(%0, stenis), %q5)]=
    1, {@prog %0=me/prog-confirm-object:
        %ch%cgWill you buy [name(%1)] for %q5 steni(s)? ('yes' or 'no')%cn},
    0, {@pemit %#=You don't even have that much money.;
        @fpose %0=tells [name(me)] to forget it. }

-
# %q6 = buyer dbref
# %q7 = object to sell dbref
# %q5 = price to be charged
# %q8 = federal sales tax
#
# %0 = yes or no

&prog-confirm-object #3345=
  @switch %0=
    yes, {@money/transfer %q5 from %q6 to [num(me)];
          @tax/federal sales/[num(me)]=%q5;
          @fpose %q6=agrees to buy [name(%q7)] and hands [name(me)] 
                 %q5 stenis.;
          &seller %q7=[num(me)];
          &buyer %q7=%q6;
          &extracted_at %q7=[secs()];
          @cleartag %q7=forsale;
          @tel %q7=loc(me);
          @resynch me;
          @fpose me=puts the [goodname(%q7)] where [name(%q6)] can 
                 take it and thanks [obj(%q6)] for [poss(%q6)] 
                 business.},
    no, {@fpose %q6=tells the merchant to forget it.},
    {@program %q6=me/prog-confirm-object:%ch%cr
              Will you buy the [goodname(%q7)] for %q5 stenis or not?  
              You must type 'yes' or 'no'.%cn}
-

          


&u-profit #3345=1.3
-
@power #3345=immutable
-

######################################################################
#                     COMMAND: @BUY <amount> <COMMODITY>             #
#                                                                    #
# This command is mostly a tool and does very little error           #
# checking.  It should be used sparingly.  The only error checking   #
# it does is whether or not the market has enough of the             #
# commodity and whether or not the buyer has enough to purchase      #
# it at the standard price.  The buyer should likely be an NPC       #
# since the goods will be transferred as accounts and no actual      #
# objects will be produced.                                          #
#                                                                    #
######################################################################


&cmd-@buy #99=$@buy * *:

  think setq(0, trim(%0, l, -));
  think setq(1, trim(%1));
  think setq(2, mul(%q0, price(%q1)));
  think setq(9, [and(isnum(%q0), iswhole(%q0))]
                [iscurrency(%q1)]
                [gte(getaccount(#26, %q1), %q0)]
                [gte(getaccount(%#, stenis), %q2)]);

  @switch/first %q9=
    
    0???, {@pemit %#=You must specify an amount that is a whole number.},
    ?0??, {@pemit %#=You failed to specify a valid currency: %q1},
    ??0?, {@pemit %#=The market doesn't have that many to sell.;
           &result-[makeattr(%q1)] %#=shortage},
    ???0, {@pemit %#=You can't afford that many at [price(%q1)] each.;
           &result-[makeattr(%q1)] %#=insufficient funds},
    1111, {@money/transfer %q2 from %# to #26;
           @account/transfer %q0 %q1 from #26 to %#;
           @pemit %#=Purchase of %q0 %q1 complete.}
-



















