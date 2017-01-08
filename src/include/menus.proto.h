#ifndef _MENUS_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN int GetSetOp(char *);
EXTERN int GetStoreBuyOp(char *);
EXTERN int GetStoreSellOp(char *);
EXTERN int GetBankOp(char *);
EXTERN void setmeup(User,char *,int);
EXTERN void SetOps(User,char *,int);
EXTERN void ShowSettings(User);
EXTERN void ShowInventory(User);
EXTERN void PrintMenu(User,int);
EXTERN void mail_menu(User,char *,int);
EXTERN void MailOps(User,char *);
EXTERN void bank(User,char *,int);
EXTERN void BankOps(User,char *,int);
EXTERN void store(User,char *,int);
EXTERN void StoreOps(User,char *,int);
#undef EXTERN
