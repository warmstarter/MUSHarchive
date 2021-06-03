/*
  Mechrep.h

  Header file for mech repair stations 
*/

/* This is the silly structure that I use for the repair stuff */
struct mechrep_data {
  dbref mynum;
  dbref tech;
  dbref current_target;
  int money;
};

/* Mech repair/type commands */
void mechrep_Rsetspeed();
void mechrep_Rsetheatsinks();
void mechrep_Rsetjumpspeed();
void mechrep_Rsettons();
void mechrep_Rsettype();
void mechrep_Rsetmove();
void mechrep_Rloadnew();
void mechrep_Rsavetemp();
void mechrep_Rsetarmor();
void mechrep_Raddweap();
void mechrep_Rrepair();
void mechrep_Rreload();
void mechrep_Raddspecial();
void mechrep_Rsetlrsrange();
void mechrep_Rsettacrange();
void mechrep_Rsetscanrange();
void mechrep_Rdisplaysection();
void mechrep_Rrestore();
void mechrep_Rshowtech();
void mechrep_Rdeltech();
void mechrep_Raddtech();

void mechrep_Rsettech();
void mechrep_Rsettarget();

/* Mem alloc/free routines */
void newfreemechrep();






