/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* packtest.c - test the basic packing routines */

#include "rodsClient.h" 

#define BANK_SERVICE_NAME               "bank"
#define NEW_ACCOUNT_OP                  "new_account"
#define DEPOSIT_OP                      "deposit"
#define BUY_BOND_OP                     "buy_bonds"
#define LIST_ACCOUNTS_OP                "list_accounts"
#define ACCOUNT_ID_KW                   "account_id"
#define ACCOUNT_TYPE_KW                 "account_type"
#define NAME_KW                         "name"
#define AMOUNT_KW                       "amount"
#define CASH_AMOUNT_KW                  "cash_amount"

int
main(int argc, char **argv)
{
    int status;
    rcComm_t *conn;
    rodsEnv myRodsEnv;
    rErrMsg_t errMsg;
    ooiGenServReqInp_t ooiGenServReqInp;
    ooiGenServReqOut_t *ooiGenServReqOut = NULL;

    status = getRodsEnv (&myRodsEnv);

    if (status < 0) {
        fprintf (stderr, "getRodsEnv error, status = %d\n", status);
        exit (1);
    }
    conn = rcConnect (myRodsEnv.rodsHost, myRodsEnv.rodsPort,
      myRodsEnv.rodsUserName, myRodsEnv.rodsZone, 0, &errMsg);

    if (conn == NULL) {
        fprintf (stderr, "rcConnect error\n");
        exit (1);
    }

    status = clientLogin(conn);
    if (status != 0) {
        fprintf (stderr, "clientLogin error\n");
       rcDisconnect(conn);
       exit (2);
    }

    bzero (&ooiGenServReqInp, sizeof (ooiGenServReqInp));
    rstrcpy (ooiGenServReqInp.servName, BANK_SERVICE_NAME, NAME_LEN);
    rstrcpy (ooiGenServReqInp.servOpr, NEW_ACCOUNT_OP, NAME_LEN);
    ooiGenServReqInp.outType = OOI_STR_TYPE;

    status = dictSetAttr (&ooiGenServReqInp.params, ACCOUNT_TYPE_KW,
      STR_MS_T, strdup ("Savings"), 0);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "main: dictSetAttr of %s error", ACCOUNT_TYPE_KW);
        exit (3);
    }
    status = dictSetAttr (&ooiGenServReqInp.params, NAME_KW,
      STR_MS_T, strdup ("Mike"), 0);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "main: dictSetAttr of %s error", NAME_KW);
        exit (4);
    }
    status = rcOoiGenServReq (conn, &ooiGenServReqInp, &ooiGenServReqOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "main: rcOoiGenServReq error");
        exit (5);
    }

    rcDisconnect(conn);

    exit (0);
}

