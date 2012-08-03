/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* tdsdir.c - test the basic routine for parsing a tds web page */

#include "rodsClient.h" 
#include "regUtil.h" 
#include <curl/curl.h>
#include <jansson.h>


#if 0
#define HLINK_PREFIX		"<a href="
#define PARENT_HLINK_DIR	"../"
typedef struct {
    int len;
    char *httpResponse;
    char *curPtr;
    CURL *easyhandle;
} httpDirStruct_t;
	
int
tdsOpendir (rsComm_t *rsComm, char *dirUrl, void **outDirPtr);
int
tdsReaddir (rsComm_t *rsComm, void *dirPtr, struct dirent *direntPtr);
int
tdsClosedir (rsComm_t *rsComm, void *dirPtr);
int
tdsStat (rsComm_t *rsComm, char *urlPath, struct stat *statbuf);
int
getNextHlink (httpDirStruct_t *httpDirStruct, char *hlink);
int
freeHttpDirStruct (httpDirStruct_t **httpDirStruct);
size_t
httpDirRespHandler (void *buffer, size_t size, size_t nmemb, void *userp);
int
listPydapDir (rsComm_t *rsComm, char *dirUrl);
#endif

#if 0
#define TDS_TOPDIR_FILE	"tds/tdsDir.xml"
#define TDS_URL		"http://hfrnet.ucsd.edu:8080/thredds/catalog.xml"
#define TDS_TOPDIR_FILE	"tds/tdsTopDir.xml"
#define TDS_URL	"http://motherlode.ucar.edu:8080/thredds/topcatalog.html"
#else
#define TDS_TOPDIR_FILE	"tds/tdsData.xml"
#define TDS_URL "http://hfrnet.ucsd.edu:8080/thredds/HFRADAR_USWC_hourly_RTV.xml"
#endif
#define THREDDS_DIR	"/thredds/"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


int
parseTopTDSDir (char *fileName, char *dirurl);
int
parseXmlDirNode (xmlDocPtr doc, xmlNodePtr mynode, char *dirurl);
int
getTDSUrl (char *dirurl, char *urlPath, char *outurl, int isData);

int
main(int argc, char **argv)
{
    int status;


    status = parseTopTDSDir (TDS_TOPDIR_FILE, TDS_URL);

    if (status < 0) {
        fprintf (stderr, "parseTopTDSDir of %s error, status = %d\n", 
          TDS_TOPDIR_FILE, status);
        exit (1);
    } 
    exit (0);
}

int
parseTopTDSDir (char *fileName, char *dirurl)
{
    xmlDocPtr doc;
    xmlNodePtr mynode;
    int status;

    doc = xmlParseFile (fileName);
    if (doc == NULL) return (-1);

    mynode = xmlDocGetRootElement (doc);

    if (mynode == NULL) {
        fprintf (stderr,"empty document\n");
        xmlFreeDoc (doc);
        return (-2);
    }
    if (xmlStrcmp(mynode->name, (const xmlChar *) "catalog")) {
        fprintf(stderr,"document of the wrong type, root node != catalog");
        xmlFreeDoc (doc);
        return(NULL);
    }
    mynode = mynode->xmlChildrenNode;
    status = parseXmlDirNode (doc, mynode, dirurl);
    xmlFreeDoc (doc);
    return 0;
}

int
parseXmlDirNode (xmlDocPtr doc, xmlNodePtr mynode, char *dirurl)
{
    xmlAttrPtr myprop;
    const xmlChar *myname, *myurlPath, *mytitle, *myhref;
    char myurl[MAX_NAME_LEN];
    int status = 0;

    if (doc == NULL || mynode == NULL) return USER__NULL_INPUT_ERR;

    while (mynode) {
        if (xmlIsBlankNode (mynode)) {
        } else if (xmlStrcmp (mynode->name, (const xmlChar *) "dataset") == 0) {
            myprop = mynode->properties;
            myname = myurlPath = mytitle = NULL;
            while (myprop) {
                if (xmlStrcmp (myprop->name, (const xmlChar *) "name") == 0) {
                    myname = myprop->children->content;
                    printf ("dataset name - %s\n", myprop->children->content);
                } else if (xmlStrcmp (myprop->name, 
                  (const xmlChar *) "urlPath") == 0) {
                    myurlPath = myprop->children->content;
                    printf ("dataset urlpath - %s\n", 
                      myprop->children->content);
               } else if (xmlStrcmp (myprop->name, (const xmlChar *) "title") 
                  == 0) {
                    mytitle = myprop->children->content;
                    printf ("dataset title - %s\n", 
                      myprop->children->content);
                }
                myprop = myprop->next;
            }
            /* drill down */
            if (myurlPath == NULL) {
                status = parseXmlDirNode (doc, mynode->children, dirurl);
            } else {
                status = getTDSUrl (dirurl, (char *) myurlPath, myurl, True);
                printf ("myurl = %s\n", myurl);
            }
        } else if (xmlStrcmp (mynode->name, (const xmlChar *) "catalogRef") 
          == 0) {
            /* this is a link */
            myprop = mynode->properties;
            myname = myhref = mytitle = NULL;
            while (myprop) {
                if (xmlStrcmp (myprop->name, (const xmlChar *) "name") == 0) {
                    myname = myprop->children->content;
                    printf ("catalogRef name - %s\n",myprop->children->content);
                } else if (xmlStrcmp (myprop->name, (const xmlChar *) "href")
                  == 0) {
                    myhref = myprop->children->content;
                    printf ("catalogRef myhref - %s\n",
                      myprop->children->content);
               } else if (xmlStrcmp (myprop->name, (const xmlChar *) "title")
                  == 0) {
                    mytitle = myprop->children->content;
                    printf ("catalogRef title - %s\n",
                      myprop->children->content);
                }
                myprop = myprop->next;
            }
            status = getTDSUrl (dirurl, (char *) myhref, myurl, False);
            printf ("myurl = %s\n", myurl);
        }
        mynode = mynode->next;
    }

    return status;
}

int
getTDSUrl (char *dirurl, char *urlPath, char *outurl, int isData)
{
    char *tmpPtr;
    int status;

    if (strncasecmp (urlPath, HTTP_PREFIX, strlen (HTTP_PREFIX)) == 0) {
        /* a full url */
        rstrcpy (outurl, urlPath, MAX_NAME_LEN);
        return 0;
    }

    if (isData == true) {
        rstrcpy (outurl, dirurl, MAX_NAME_LEN);
        tmpPtr = strcasestr (outurl, THREDDS_DIR);
        tmpPtr += strlen (THREDDS_DIR);
        snprintf (tmpPtr, MAX_NAME_LEN, "dodsC/%s", urlPath);
        return 0;
    }
    /* a link */    
    if (strncasecmp (urlPath, THREDDS_DIR, strlen (THREDDS_DIR)) == 0) {
        rstrcpy (outurl, dirurl, MAX_NAME_LEN);
        tmpPtr = strcasestr (outurl, THREDDS_DIR);
        /* start with /thredds/ */
        if (tmpPtr == NULL) return -1;
        snprintf (tmpPtr, MAX_NAME_LEN, "%s", urlPath);
        return 0;
    } else {
        char myFile[MAX_NAME_LEN], mydir[MAX_NAME_LEN];
        status = splitPathByKey (dirurl, mydir, myFile, '/');
        if (status < 0) return status;
        snprintf (outurl, MAX_NAME_LEN, "%s/%s", mydir, urlPath);
        return 0;
    }
}

#if 0
int
listPydapDir (rsComm_t *rsComm, char *dirUrl)
{
    struct dirent dirent;
    int status;
    httpDirStruct_t *httpDirStruct = NULL;

    status = tdsOpendir (rsComm,  dirUrl, (void **) &httpDirStruct);
    if (status < 0) {
	fprintf (stderr, "tdsOpendir of %s error, status = %d\n", 
          dirUrl, status);
        return status;
    }
    while (tdsReaddir (rsComm, httpDirStruct, &dirent) >= 0) {
        char childUrl[MAX_NAME_LEN]; 
        struct stat statbuf;

        snprintf (childUrl, MAX_NAME_LEN, "%s%s", dirUrl, dirent.d_name);
        status = tdsStat (rsComm, childUrl, &statbuf);
        if (status < 0) {
            fprintf (stderr, "tdsStat of %s error, status = %d\n",
              childUrl, status);
            return status;
        }
        printf ("child: %s\n", childUrl);
	if ((statbuf.st_mode & S_IFDIR) != 0) {
            status = listPydapDir (rsComm, childUrl);
            if (status < 0) {
                fprintf (stderr, "listPydapDir of %s error, status = %d\n",
                  childUrl, status);
                tdsClosedir (rsComm, httpDirStruct);
                return status;
            }
        }
    }
    status = tdsClosedir (rsComm, httpDirStruct);

    if (status < 0) {
        fprintf (stderr, "tdsClosedir of %s error, status = %d\n",
          dirUrl, status);
    }
    return status;
}

int
tdsOpendir (rsComm_t *rsComm, char *dirUrl, void **outDirPtr)
{
    CURLcode res;
    CURL *easyhandle;
    httpDirStruct_t *httpDirStruct = NULL;

    if (dirUrl == NULL || outDirPtr == NULL) return USER__NULL_INPUT_ERR;
    
    *outDirPtr = NULL;
    easyhandle = curl_easy_init();
    if(!easyhandle) {
        rodsLog (LOG_ERROR,
          "httpDirRespHandler: curl_easy_init error");
        return OOI_CURL_EASY_INIT_ERR;
    } 
    curl_easy_setopt(easyhandle, CURLOPT_URL, dirUrl);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, httpDirRespHandler);
    httpDirStruct = (httpDirStruct_t *) calloc (1, sizeof (httpDirStruct_t));
    httpDirStruct->easyhandle = easyhandle;
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, httpDirStruct);
    /* this is needed for ERDDAP site */
    curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1);

    res = curl_easy_perform (easyhandle);

    if (res != CURLE_OK) {
        /* res is +ive for error */
        rodsLog (LOG_ERROR,
          "httpDirRespHandler: curl_easy_perform error: %d", res);
        freeHttpDirStruct (&httpDirStruct);
        curl_easy_cleanup (easyhandle);
        return OOI_CURL_EASY_PERFORM_ERR - res;
    }
    *outDirPtr = httpDirStruct;
    return 0;
}

int
tdsReaddir (rsComm_t *rsComm, void *dirPtr, struct dirent *direntPtr)
{
    char hlink[MAX_NAME_LEN];
    int status, len;
    char *ptr;
    httpDirStruct_t *httpDirStruct = (httpDirStruct_t *) dirPtr;

    while ((status = getNextHlink (httpDirStruct, hlink)) >= 0) {

        if (strcmp (hlink, PARENT_HLINK_DIR) == 0) continue;
        if (strncmp (hlink, HTTP_PREFIX, strlen (HTTP_PREFIX)) == 0) continue;
        /* end with .html ? */
        len = strlen (hlink);
        ptr = hlink +len - 5;
        if (strcmp (ptr, ".html") == 0) continue;
        rstrcpy (direntPtr->d_name, hlink, MAX_NAME_LEN);
        break;
    }
    return status;
}

int
getNextHlink (httpDirStruct_t *httpDirStruct, char *hlink)
{
    char *ptr, *endPtr;

    ptr = strcasestr (httpDirStruct->curPtr, HLINK_PREFIX);
    if (ptr == NULL) return -1;
    ptr += strlen (HLINK_PREFIX);
    ptr = strchr (ptr, '\"');
    if (ptr == NULL) return -1;
    ptr++;
    endPtr = strchr (ptr, '\"');
    if (endPtr == NULL) return -1;
    *endPtr = '\0';
    rstrcpy (hlink, ptr, MAX_NAME_LEN);
    *endPtr = '\"';
    httpDirStruct->curPtr = endPtr + 1;

    return 0;
}

int
freeHttpDirStruct (httpDirStruct_t **httpDirStruct)
{
    if (httpDirStruct == NULL || *httpDirStruct == NULL) return 0;

    if ((*httpDirStruct)->httpResponse != NULL) 
        free ((*httpDirStruct)->httpResponse);
    free (*httpDirStruct);

    return 0;
}

size_t
httpDirRespHandler (void *buffer, size_t size, size_t nmemb, void *userp)
{
    httpDirStruct_t *httpDirStruct = (httpDirStruct_t *) userp;
    
    char *newHttpResponse;
    int newLen;

    int len = size * nmemb;
    
    if (httpDirStruct->len > 0) {
	newLen = httpDirStruct->len + len;
        newHttpResponse = (char *) calloc (1, newLen);
        memcpy (newHttpResponse, httpDirStruct->httpResponse, 
          httpDirStruct->len);
        memcpy (newHttpResponse + httpDirStruct->len, buffer, len);
        httpDirStruct->len = newLen;
        free (httpDirStruct->httpResponse);
        httpDirStruct->httpResponse = newHttpResponse;
        httpDirStruct->curPtr = newHttpResponse;
    } else {
        newHttpResponse = (char *) calloc (1, len);
        memcpy (newHttpResponse, buffer, len);
        httpDirStruct->len = len;
    }
    httpDirStruct->httpResponse = newHttpResponse;
    httpDirStruct->curPtr = newHttpResponse;

    return len;
}

int
tdsClosedir (rsComm_t *rsComm, void *dirPtr)
{
    httpDirStruct_t *httpDirStruct = (httpDirStruct_t *) dirPtr;

    if (httpDirStruct == NULL) return 0;

    if (httpDirStruct->easyhandle != NULL) {
        curl_easy_cleanup (httpDirStruct->easyhandle);
    }
    freeHttpDirStruct (&httpDirStruct);

    return 0;
}

int
tdsStat (rsComm_t *rsComm, char *urlPath, struct stat *statbuf)
{
    int len;

    if (urlPath == NULL || statbuf == NULL) return USER__NULL_INPUT_ERR;
    bzero (statbuf, sizeof (struct stat));
    len = strlen (urlPath);
    /* end with "/" ? */
    if (urlPath[len - 1] == '/') {
        statbuf->st_mode = DEFAULT_DIR_MODE | S_IFDIR;
    } else {
        statbuf->st_mode = DEFAULT_FILE_MODE | S_IFREG;
        statbuf->st_size = -1;
    }
    return (0);
}
#endif
