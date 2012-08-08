/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* tdsDriver.c - the tds driver */

#include "rodsClient.h" 
#include "regUtil.h" 
#include "tdsDriver.h" 

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
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, tdsDirRespHandler);
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
    free (tdsDirStruct->httpResponse);          /* don't need this anymore */
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
    rstrcpy (tdsDirStruct->dirUrl, dirUrl, MAX_NAME_LEN);
    *outDirPtr = tdsDirStruct;
    return 0;
}

int
tdsReaddir (rsComm_t *rsComm, void *dirPtr, struct dirent *direntPtr)
{
    int status = -1;
    tdsDirStruct_t *tdsDirStruct = (tdsDirStruct_t *) dirPtr;
    xmlAttrPtr myprop;
    const xmlChar *myname, *myurlPath, *mytitle, *myhref, *tmpname;
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
                if ((char *)myname != NULL && strlen ((char *)myname) > 0) {
                    tmpname = myname;
                } else if ((char *)mytitle != NULL &&
                  strlen ((char *)mytitle) > 0) {
                    tmpname = mytitle;
                } else {
                    rodsLog (LOG_ERROR,
                      "tdsReaddir: dataset %s has no name nor title",
                      myurlPath);
                    continue;
                }
                rstrcpy (direntPtr->d_name, (char *) tmpname, NAME_MAX);
                status = setTDSUrl (tdsDirStruct, (char *)myurlPath, False);
                if (status >= 0) direntPtr->d_ino = status;
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
            if (myhref == NULL) continue;
            if ((char *)myname != NULL && strlen ((char *)myname) > 0) {
                tmpname = myname;
            } else if ((char *)mytitle != NULL &&
              strlen ((char *)mytitle) > 0) {
                tmpname = mytitle;
            } else {
                rodsLog (LOG_ERROR,
                  "tdsReaddir: dataset %s has no name nor title",
                  myurlPath);
                continue;
            }
            snprintf (direntPtr->d_name, NAME_MAX, "%s", (char *) tmpname);
            status = setTDSUrl (tdsDirStruct, (char *)myhref, True);
            if (status >= 0) direntPtr->d_ino = status;
            return status;
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
tdsDirRespHandler (void *buffer, size_t size, size_t nmemb, void *userp)
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
        statbuf->st_size = UNKNOWN_FILE_SZ;
    }
    return (0);
}

int
setTDSUrl (tdsDirStruct_t *tdsDirStruct, char *urlPath, int isDir)
{
    int inx;
    char *tmpPtr, *outurl;
    int status;

    inx = allocTdsUrlPath (tdsDirStruct);
    if (inx < 0) return inx;

    if (isDir == True) {
        tdsDirStruct->urlPath[inx].st_mode = S_IFDIR;
    } else {
        tdsDirStruct->urlPath[inx].st_mode = S_IFREG;
    }
    outurl = tdsDirStruct->urlPath[inx].path;
    if (strncasecmp (urlPath, HTTP_PREFIX, strlen (HTTP_PREFIX)) == 0) {
        /* a full url */
        rstrcpy (outurl, urlPath, MAX_NAME_LEN);
        return inx;
    }

    if (isDir == False) {
        rstrcpy (outurl, tdsDirStruct->dirUrl, MAX_NAME_LEN);
        tmpPtr = strcasestr (outurl, THREDDS_DIR);
        tmpPtr += strlen (THREDDS_DIR);
        snprintf (tmpPtr, MAX_NAME_LEN, "dodsC/%s", urlPath);
        return inx;
    }
    /* a link */
    if (strncasecmp (urlPath, THREDDS_DIR, strlen (THREDDS_DIR)) == 0) {
        rstrcpy (outurl, tdsDirStruct->dirUrl, MAX_NAME_LEN);
        tmpPtr = strcasestr (outurl, THREDDS_DIR);
        /* start with /thredds/ */
        if (tmpPtr == NULL) return -1;
        snprintf (tmpPtr, MAX_NAME_LEN, "%s", urlPath);
        return inx;
    } else {
        char myFile[MAX_NAME_LEN], mydir[MAX_NAME_LEN];
        status = splitPathByKey (tdsDirStruct->dirUrl, mydir, myFile, '/');
        if (status < 0) return status;
        snprintf (outurl, MAX_NAME_LEN, "%s/%s", mydir, urlPath);
        return inx;
    }
}

int
allocTdsUrlPath (tdsDirStruct_t *tdsDirStruct)
{
    int i;

    for (i = 0; i < NUM_URL_PATH; i++) {
        if (tdsDirStruct->urlPath[i].inuse == False) {
            tdsDirStruct->urlPath[i].inuse = True;
            return i;
        }
    }
    return OUT_OF_URL_PATH;
}

int
freeTdsUrlPath (tdsDirStruct_t *tdsDirStruct, int inx)
{
    if (inx < 0 || inx >= NUM_URL_PATH) return URL_PATH_INX_OUT_OF_RANGE;

    tdsDirStruct->urlPath[inx].inuse = False;
    return 0;
}

int
tdsStageToCache (rsComm_t *rsComm, fileDriverType_t cacheFileType,
int mode, int flags, char *urlPath, char *cacheFilename, rodsLong_t dataSize,
keyValPair_t *condInput)
{
    return 0;
}

#if 0
/* tdsStageToCache - use HTTP GET to get the data */
int
tdsStageToCache (rsComm_t *rsComm, fileDriverType_t cacheFileType,
int mode, int flags, char *urlPath, char *cacheFilename, rodsLong_t dataSize,
keyValPair_t *condInput)
{
    CURL *easyhandle;
    CURLcode res;
    httpDownloadStruct_t httpDownloadStruct;

    easyhandle = curl_easy_init();
    if(!easyhandle) {
        rodsLog (LOG_ERROR,
          "tdsStageToCache: curl_easy_init error for %s", urlPath);
        return OOI_CURL_EASY_INIT_ERR;
    }
    curl_easy_setopt(easyhandle, CURLOPT_URL, urlPath);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, httpDownloadFunc);
    bzero (&httpDownloadStruct, sizeof (httpDownloadStruct));
    rstrcpy (httpDownloadStruct.outfile, cacheFilename, MAX_NAME_LEN);
    httpDownloadStruct.outFd = -1;
    httpDownloadStruct.mode = mode;
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &httpDownloadStruct);
    /* this is needed for ERDDAP site */
    curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1);

    res = curl_easy_perform (easyhandle);
    if (httpDownloadStruct.outFd > 0) close (httpDownloadStruct.outFd);

    return 0;

}

int
httpDownloadFunc (void *buffer, size_t size, size_t nmemb, void *userp)
{
    httpDownloadStruct_t *httpDownloadStruct = (httpDownloadStruct_t *) userp;
    rodsLong_t len = size * nmemb;
    rodsLong_t bytesWriten;


    if (httpDownloadStruct->outFd < 0) {
        httpDownloadStruct->outFd = open (httpDownloadStruct->outfile,
          O_WRONLY | O_CREAT | O_TRUNC, httpDownloadStruct->mode);
        if (httpDownloadStruct->outFd < 0) { /* error */
            rodsLog (LOG_ERROR,
            "httpDownloadFunc: cannot open file %s, errno = %d",
              httpDownloadStruct->outfile, errno);
            return (httpDownloadStruct->outFd);
        }
    }
    bytesWriten = myWrite (httpDownloadStruct->outFd, buffer, len, 
      FILE_DESC_TYPE, NULL);
    if (bytesWriten != len) {
        rodsLog (LOG_ERROR,
        "httpDownloadFunc: bytesWriten %ld does not match len %ld",
          bytesWriten, len);
        return (SYS_COPY_LEN_ERR);
    }
    httpDownloadStruct->len += bytesWriten;

    return len;
}
#endif

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
    while (tdsReaddir (rsComm, tdsDirStruct, &dirent) >= 0) {
        char childUrl[MAX_NAME_LEN];
        int st_mode;

        rstrcpy (childUrl, tdsDirStruct->urlPath[dirent.d_ino].path,
          MAX_NAME_LEN);
        st_mode = tdsDirStruct->urlPath[dirent.d_ino].st_mode;
        freeTdsUrlPath (tdsDirStruct, dirent.d_ino);
        if ((st_mode & S_IFDIR) != 0) {
            printf ("dir child : name = %s, URL = %s\n", dirent.d_name,
              childUrl);
            status = listTdsDir (rsComm, childUrl);
            if (status < 0) {
                fprintf (stderr, "listTdsDir of %s error, status = %d\n",
                  childUrl, status);
                tdsClosedir (rsComm, tdsDirStruct);
                return status;
            }
        } else {
            printf ("child : name = %s, URL = %s\n", dirent.d_name, childUrl);
        }
    }
    status = tdsClosedir (rsComm, tdsDirStruct);

    if (status < 0) {
        fprintf (stderr, "tdsClosedir of %s error, status = %d\n",
          dirUrl, status);
    }
    return status;
}

