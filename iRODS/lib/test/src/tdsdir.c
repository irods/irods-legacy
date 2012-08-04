/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* tdsdir.c - test the basic routine for parsing a tds web page */

#include "rodsClient.h" 
#include "regUtil.h" 
#include <curl/curl.h>
#include <jansson.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#if 0
#define TDS_TOPDIR_FILE "tds/tdsDir.xml"
#define TDS_URL         "http://hfrnet.ucsd.edu:8080/thredds/catalog.xml"
#define TDS_TOPDIR_FILE "tds/tdsTopDir.xml"
#define TDS_URL "http://motherlode.ucar.edu:8080/thredds/topcatalog.html"
#else
#define TDS_TOPDIR_FILE "tds/tdsData.xml"
#define TDS_URL "http://hfrnet.ucsd.edu:8080/thredds/HFRADAR_USWC_hourly_RTV.xml"
#endif
#define THREDDS_DIR     "/thredds/"


typedef struct {
    int len;
    char *httpResponse;
    xmlDocPtr doc; 
    xmlNodePtr rootnode; 
    xmlNodePtr curnode; 
    char dirurl[MAX_NAME_LEN];
    CURL *easyhandle;
} tdsDirStruct_t;
	
int
tdsOpendir (rsComm_t *rsComm, char *dirUrl, void **outDirPtr);
int
tdsReaddir (rsComm_t *rsComm, void *dirPtr, struct dirent *direntPtr);
int
tdsClosedir (rsComm_t *rsComm, void *dirPtr);
int
tdsStat (rsComm_t *rsComm, char *urlPath, struct stat *statbuf);
int
getNextNode (tdsDirStruct_t *tdsDirStruct);
int
freeTdsDirStruct (tdsDirStruct_t **tdsDirStruct);
size_t
httpDirRespHandler (void *buffer, size_t size, size_t nmemb, void *userp);
int
listTdsDir (rsComm_t *rsComm, char *dirUrl);

int
parseTopTDSDir (char *fileName, char *dirurl);
int
parseXmlDirNode (xmlDocPtr doc, xmlNodePtr mynode, char *dirurl);
int
getTDSUrl (char *dirurl, char *urlPath, char *outurl, int isDir);
int
setTdsDirentName (char *myname, char *mytitle, char *myurlPath, int isDir,
char *outPath);

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
    status = listTdsDir (NULL, TDS_URL);
    if (status < 0) {
        fprintf (stderr, "listTdsDir of %s error, status = %d\n",
          TDS_URL, status);
        exit (2);
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
                status = getTDSUrl (dirurl, (char *) myurlPath, myurl, False);
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
            status = getTDSUrl (dirurl, (char *) myhref, myurl, True);
            printf ("myurl = %s\n", myurl);
        }
        mynode = mynode->next;
    }

    return status;
}

int
getTDSUrl (char *dirurl, char *urlPath, char *outurl, int isDir)
{
    char *tmpPtr;
    int status;

    if (strncasecmp (urlPath, HTTP_PREFIX, strlen (HTTP_PREFIX)) == 0) {
        /* a full url */
        rstrcpy (outurl, urlPath, MAX_NAME_LEN);
        return 0;
    }

    if (isDir == False) {
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

int
listTdsDir (rsComm_t *rsComm, char *dirUrl)
{
    struct dirent dirent;
    int status;
    tdsDirStruct_t *tdsDirStruct = NULL;

    status = tdsOpendir (rsComm,  dirUrl, (void **) &tdsDirStruct);
    if (status < 0) {
	fprintf (stderr, "tdsOpendir of %s error, status = %d\n", 
          dirUrl, status);
        return status;
    }
#if 0
    while (tdsReaddir (rsComm, tdsDirStruct, &dirent) >= 0) {
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
            status = listTdsDir (rsComm, childUrl);
            if (status < 0) {
                fprintf (stderr, "listTdsDir of %s error, status = %d\n",
                  childUrl, status);
                tdsClosedir (rsComm, tdsDirStruct);
                return status;
            }
        }
    }
    status = tdsClosedir (rsComm, tdsDirStruct);

    if (status < 0) {
        fprintf (stderr, "tdsClosedir of %s error, status = %d\n",
          dirUrl, status);
    }
#endif
    return status;
}

int
tdsOpendir (rsComm_t *rsComm, char *dirUrl, void **outDirPtr)
{
    CURLcode res;
    CURL *easyhandle;
    tdsDirStruct_t *tdsDirStruct = NULL;

    if (dirUrl == NULL || outDirPtr == NULL) return USER__NULL_INPUT_ERR;
    
    *outDirPtr = NULL;
    easyhandle = curl_easy_init();
    if(!easyhandle) {
        rodsLog (LOG_ERROR,
          "tdsOpendir: curl_easy_init error");
        return OOI_CURL_EASY_INIT_ERR;
    } 
    curl_easy_setopt(easyhandle, CURLOPT_URL, dirUrl);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, httpDirRespHandler);
    tdsDirStruct = (tdsDirStruct_t *) calloc (1, sizeof (tdsDirStruct_t));
    tdsDirStruct->easyhandle = easyhandle;
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, tdsDirStruct);
    /* this is needed for ERDDAP site */
    curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1);

    res = curl_easy_perform (easyhandle);

    if (res != CURLE_OK) {
        /* res is +ive for error */
        rodsLog (LOG_ERROR,
          "tdsOpendir: curl_easy_perform error: %d", res);
        freeTdsDirStruct (&tdsDirStruct);
        curl_easy_cleanup (easyhandle);
        return OOI_CURL_EASY_PERFORM_ERR - res;
    }
    tdsDirStruct->doc = xmlParseMemory (tdsDirStruct->httpResponse,
      tdsDirStruct->len);
    if (tdsDirStruct->doc == NULL) {
        freeTdsDirStruct (&tdsDirStruct);
        rodsLog (LOG_ERROR,
          "tdsOpendir: xmlParseMemory error for %s", dirUrl);
        return XML_PARSING_ERR;
    }
    free (tdsDirStruct->httpResponse);		/* don't need this anymore */
    tdsDirStruct->httpResponse = NULL;
    tdsDirStruct->rootnode = xmlDocGetRootElement (tdsDirStruct->doc);

    if (tdsDirStruct->rootnode == NULL) {
        rodsLog (LOG_ERROR,
          "tdsOpendir: xmlDocGetRootElement error for %s", dirUrl);
        freeTdsDirStruct (&tdsDirStruct);
        return XML_PARSING_ERR;
    }
    if (xmlStrcmp(tdsDirStruct->rootnode->name, (const xmlChar *) "catalog")) {
        rodsLog (LOG_ERROR,
          "tdsOpendir: root node name %s is not 'catalog' for %s", 
          tdsDirStruct->rootnode, dirUrl);
        freeTdsDirStruct (&tdsDirStruct);
        return XML_PARSING_ERR;
    }
    tdsDirStruct->curnode = tdsDirStruct->rootnode;
    *outDirPtr = tdsDirStruct;
    return 0;
}

int
tdsReaddir (rsComm_t *rsComm, void *dirPtr, struct dirent *direntPtr)
{
    int status;
    tdsDirStruct_t *tdsDirStruct = (tdsDirStruct_t *) dirPtr;
    xmlAttrPtr myprop;
    const xmlChar *myname, *myurlPath, *mytitle, *myhref;
    xmlNodePtr mynode;

    while (getNextNode (tdsDirStruct)) {
        mynode = tdsDirStruct->curnode;
        if (xmlIsBlankNode (tdsDirStruct->curnode)) {
            continue;
        } else if (xmlStrcmp (mynode->name, (const xmlChar *) "dataset") == 0) {
            myprop = mynode->properties;
            myname = myurlPath = mytitle = NULL;
            while (myprop) {
                if (xmlStrcmp (myprop->name, (const xmlChar *) "name") == 0) {
                    myname = myprop->children->content;
                } else if (xmlStrcmp (myprop->name,
                  (const xmlChar *) "urlPath") == 0) {
                    myurlPath = myprop->children->content;
               } else if (xmlStrcmp (myprop->name, (const xmlChar *) "title")
                  == 0) {
                    mytitle = myprop->children->content;
                }
                myprop = myprop->next;
            }
            if (myurlPath == NULL) {
                /* drill down */
                continue;
            } else {
                status = setTdsDirentName ((char *)myname, (char *) mytitle,
                  (char *)  myurlPath, False, direntPtr->d_name);
                return status;
            }
        } else if (xmlStrcmp (mynode->name, (const xmlChar *) "catalogRef")
          == 0) {
            /* this is a link */
            myprop = mynode->properties;
            myname = myhref = mytitle = NULL;
            while (myprop) {
                if (xmlStrcmp (myprop->name, (const xmlChar *) "name") == 0) {
                    myname = myprop->children->content;
                } else if (xmlStrcmp (myprop->name, (const xmlChar *) "href")
                  == 0) {
                    myhref = myprop->children->content;
               } else if (xmlStrcmp (myprop->name, (const xmlChar *) "title")
                  == 0) {
                    mytitle = myprop->children->content;
                }
                myprop = myprop->next;
            }
            status = setTdsDirentName ((char *) myname, (char *) mytitle, 
              (char *) myhref, True, direntPtr->d_name);
        }
    }
    return status;
}

int
getNextNode (tdsDirStruct_t *tdsDirStruct)
{
    if (tdsDirStruct == NULL || tdsDirStruct->curnode == NULL) return False;
    if (tdsDirStruct->curnode == tdsDirStruct->rootnode) {
        tdsDirStruct->curnode = tdsDirStruct->curnode->children;
        if (tdsDirStruct->curnode == NULL) {
             return False;
        } else {
             return True;
        }
    } else if (tdsDirStruct->curnode->children != NULL) {
        tdsDirStruct->curnode = tdsDirStruct->curnode->children;
        return True;
    } else if (tdsDirStruct->curnode->next != NULL) {
        tdsDirStruct->curnode = tdsDirStruct->curnode->next;
        return True;
    }

    tdsDirStruct->curnode = tdsDirStruct->curnode->parent;
    while (tdsDirStruct->curnode != tdsDirStruct->rootnode) {
        if (tdsDirStruct->curnode->next != NULL) {
            tdsDirStruct->curnode = tdsDirStruct->curnode->next;
            return True;
        }
        tdsDirStruct->curnode = tdsDirStruct->curnode->parent; 
    }
    /* we ran out */
    tdsDirStruct->curnode = NULL;
    return False;
}

int
setTdsDirentName (char *myname, char *mytitle, char *myurlPath, int isDir,
char *outPath)
{
    char *tmpname = NULL;
    int namelen, urlPathlen;

    if ((urlPathlen = strlen (myurlPath)) > NAME_MAX + isDir) {
        rodsLog (LOG_ERROR,
          "setTdsDirentName: urlPathlen of %s too long", myurlPath);
        return USER_STRLEN_TOOLONG;
    }
    if (myname != NULL && (namelen = strlen (myname)) > 0) {
        tmpname = myname;
    } else if (mytitle != NULL && (namelen = strlen (mytitle)) > 0) {
        tmpname = mytitle;
    }
    if (tmpname != NULL && namelen + urlPathlen + 4 + isDir < NAME_MAX) {
        if (isDir == True) {
            snprintf (outPath, NAME_MAX, "%s++++%s/", tmpname, myurlPath);
        } else {
            snprintf (outPath, NAME_MAX, "%s++++%s", tmpname, myurlPath);
        }
    } else {
        if (isDir == True) {
            snprintf (outPath, NAME_MAX, "%s/", myurlPath);
        } else {
            snprintf (outPath, NAME_MAX, "%s", myurlPath);
        }
    }
    return 0;
}

int
freeTdsDirStruct (tdsDirStruct_t **tdsDirStruct)
{
    if (tdsDirStruct == NULL || *tdsDirStruct == NULL) return 0;

    if ((*tdsDirStruct)->httpResponse != NULL) 
        free ((*tdsDirStruct)->httpResponse);
    if ((*tdsDirStruct)->doc != NULL)
        xmlFreeDoc ((*tdsDirStruct)->doc);
    free (*tdsDirStruct);

    return 0;
}

size_t
httpDirRespHandler (void *buffer, size_t size, size_t nmemb, void *userp)
{
    tdsDirStruct_t *tdsDirStruct = (tdsDirStruct_t *) userp;
    
    char *newHttpResponse;
    int newLen;

    int len = size * nmemb;
    
    if (tdsDirStruct->len > 0) {
	newLen = tdsDirStruct->len + len;
        newHttpResponse = (char *) calloc (1, newLen);
        memcpy (newHttpResponse, tdsDirStruct->httpResponse, 
          tdsDirStruct->len);
        memcpy (newHttpResponse + tdsDirStruct->len, buffer, len);
        tdsDirStruct->len = newLen;
        free (tdsDirStruct->httpResponse);
        tdsDirStruct->httpResponse = newHttpResponse;
    } else {
        newHttpResponse = (char *) calloc (1, len);
        memcpy (newHttpResponse, buffer, len);
        tdsDirStruct->len = len;
    }
    tdsDirStruct->httpResponse = newHttpResponse;

    return len;
}

int
tdsClosedir (rsComm_t *rsComm, void *dirPtr)
{
    tdsDirStruct_t *tdsDirStruct = (tdsDirStruct_t *) dirPtr;

    if (tdsDirStruct == NULL) return 0;

    if (tdsDirStruct->easyhandle != NULL) {
        curl_easy_cleanup (tdsDirStruct->easyhandle);
    }
    freeTdsDirStruct (&tdsDirStruct);

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
