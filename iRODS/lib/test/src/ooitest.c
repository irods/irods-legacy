/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* packtest.c - test the basic packing routines */

#include "rodsClient.h" 
#include <curl/curl.h>
#include <jansson.h>

#define NEW_ACC_PAYLOAD "payload={\"serviceRequest\": { \"serviceName\": \"bank\", \"serviceOp\": \"new_account\", \"params\": { \"name\": \"kurt\" }}}"

#define DEPOS_PAYLOAD "payload={\"serviceRequest\": { \"serviceName\": \"bank\", \"serviceOp\": \"deposit\", \"params\": { \"account_id\": \"1c231090c69b4eceae1c469f85da10ea\", \"amount\": 125.00 }}}"

#define BUY_BOND_PAYLOAD "payload={\"serviceRequest\": { \"serviceName\": \"bank\", \"serviceOp\": \"buy_bonds\", \"params\": { \"account_id\": \"1c231090c69b4eceae1c469f85da10ea\", \"cash_amount\": 115.00 }}}"

#define OOI_GATEWAY_URL "http://localhost"
#define OOI_GATEWAY_PORT 		"5000"

#define ION_SERVICE			"ion-service"
#define BANK_SERVICE_NAME    		"bank"
#define NEW_ACCOUNT_OP			"new_account"
#define DEPOSIT_OP			"deposit"
#define BUY_BOND_OP			"buy_bonds"
#define ACCOUNT_ID_KW			"account_id"
#define NAME_KW				"name"
#define AMOUNT_KW			"amount"
#define CASH_AMOUNT_KW			"cash_amount"

typedef struct {
    char account_id[NAME_LEN];
    char reponseStr[MAX_NAME_LEN];
    float cashDeposit;
    float bondAmount;
} bankOprOut_t;


int 
runBankTest ();
int
doDeposit (CURL *easyhandle, char *account_id, float ammount);
size_t
decodeDepositOut (void *buffer, size_t size, size_t nmemb, void *userp);

int
main(int argc, char **argv)
{
    int status;

    status = runBankTest ();

    if (status < 0)
        exit (0);
    else
	exit (1);
}

int
runBankTest ()
{
    CURL *easyhandle;
    CURLcode res;
    char myUrl[MAX_NAME_LEN];
    int status;

    easyhandle = curl_easy_init();
    if(!easyhandle) {
        printf("Curl Error: Initialization failed\n");
        return(-1);
    } 
    status = doDeposit (easyhandle, "1c231090c69b4eceae1c469f85da10ea", 125.0);

    return status;
}

int
doDeposit (CURL *easyhandle, char *account_id, float ammount)
{
    json_t *obj;
    CURLcode res;
    char myUrl[MAX_NAME_LEN];
    char postStr[MAX_NAME_LEN];
    char *objStr;
    bankOprOut_t bankOprOut;

    obj = json_pack ("{s:{s:s,s:s,s:{s:s,s:f}}}",
                          "serviceRequest",
                          "serviceName", BANK_SERVICE_NAME,
                          "serviceOp", DEPOSIT_OP,
                          "params",
                          ACCOUNT_ID_KW, account_id,
                          AMOUNT_KW, 125.00); 
    if (obj == NULL) {
        rodsLog (LOG_ERROR, "doDeposit: json_pack error");
        return -2;
    }
	
    objStr = json_dumps (obj, 0);

    if (objStr == NULL) {
        rodsLog (LOG_ERROR, "doDeposit: json_dumps error");
        return -2;
    }

    snprintf (postStr, MAX_NAME_LEN, "payload=%s", objStr);
    /* deposit */
    snprintf (myUrl, MAX_NAME_LEN, "%s:%s/%s/%s/%s", 
      OOI_GATEWAY_URL, OOI_GATEWAY_PORT, ION_SERVICE, BANK_SERVICE_NAME, 
      DEPOSIT_OP);
    printf ("%s\n", myUrl);

    curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, postStr);
    curl_easy_setopt(easyhandle, CURLOPT_URL, myUrl);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, decodeDepositOut); 
    bzero (&bankOprOut, sizeof (bankOprOut));
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &bankOprOut); 

    res = curl_easy_perform(easyhandle); 
    free (obj);
    free (objStr);
    if (res != CURLE_OK) {
        rodsLog (LOG_ERROR, "runBankTest: deposit error: %d", res);
        return -1;
    }
    printf ("output = %s\n", bankOprOut.reponseStr);

    return 0;
}
#if 0
    /* buy_bonds */
    snprintf (myUrl, MAX_NAME_LEN, "%s:%s/%s/%s/%s", 
      OOI_GATEWAY_URL, OOI_GATEWAY_PORT, ION_SERVICE, BANK_SERVICE_NAME, 
      BUY_BOND_OP_KW);

    curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, BUY_BOND_PAYLOAD); 
    curl_easy_setopt(easyhandle, CURLOPT_URL, myUrl);

    res = curl_easy_perform(easyhandle); 
    if (res != CURLE_OK) {
        rodsLog (LOG_ERROR, "runBankTest: buy_bonds error: %d", res);
        return -1;
    }
    return 0;
}
#endif

size_t
decodeDepositOut (void *buffer, size_t size, size_t nmemb, void *userp)
{
    json_t *root, *dataObj, *responseObj;
    json_error_t jerror;
    const char *responseStr;
    bankOprOut_t *bankOprOut;

    root = json_loads((const char*) buffer, 0, &jerror);
    if (!root) {
        rodsLog (LOG_ERROR,
          "decodeDepositOut: json_loads error. %s", jerror.text);
        return 0;
    } 
    dataObj = json_object_get(root, "data");
    if (!dataObj) {
       rodsLog (LOG_ERROR,
          "decodeDepositOut: json_object_get data failed.");
	free (root);
        return 0;
    }
    responseObj = json_object_get(dataObj, "GatewayResponse");
    if (!responseObj) {
       rodsLog (LOG_ERROR,
          "decodeDepositOut: json_object_get GatewayResponse failed.");
	free (root);
        return 0;
    }
    responseStr = json_string_value (responseObj);
    bankOprOut = (bankOprOut_t *) userp;
    strncpy (bankOprOut->reponseStr, responseStr, MAX_NAME_LEN);
    free (root);

    return nmemb*size;
}
