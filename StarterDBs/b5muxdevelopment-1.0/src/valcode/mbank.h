/* Mike's bank code */


/* Bank overview 

	Commands: 

	BankAdm:
		BankLoad
		BankSave
		AcctLoad
		AcctSave
		BankInit
		AcctInit
		BankStat
		ShowAcct
	BankMgr:
		AcctCreate
		AcctShow
		AcctLock
		AcctUnlock
		AcctDelete
		SetPIN		
	Bank:
		Access
		Withdrawl
		Display
		Deposit
		ShowCredit
		ChangePIN
*/
struct s_Bank {

int id;
int pid;
int cur; /* currency bank operates on */
int owner; /* Theoretical owner of Bank */
double ir;
double money; /* how much the bank has in money */
double accts; /* What the bank has in accounts */ 
double loans; /* What the bank has in loans */ 
int nloans; /* Number of loans the bank has outstanding*/
int naccts; /* number of accts the bank has */ 
int bobj;  /* Object of the bank */ 
int period;  /* Object of the bank */ 
int flags;  /* Flags attrib */
};

struct s_Acct {
  int num;
  int bank;
  int player;
  int pin;
  int cur;
  double ir; 
  double amt;
  int flags;
  int expire;
};

typedef struct s_Acct t_Acct;	


typedef struct s_Bank t_Bank;
#define NUMBANK 25
#define BDBVERS 1

t_Bank Banks[NUMBANK];
int NumBanks;
void SaveBankDataToFile();
void LoadBankDataFromFile();


static char *rcsmbankh="$Id: mbank.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
