contains(*list, *elem) {
    *ret = false;
    foreach(*e in *list) {
        if(*e == *elem) {
            *ret = true;
        }
    }
    *ret;
}

 
createCollections(*coll, *cs) {
    foreach(*c in *cs) {
        msiCollCreate(/*coll/*c, "1", *status);
    }
}

# Consturct a list *cs which has every file under localRoot as its elements
getFiles(*localRoot, *localPaths) {
    *cs = list();
    *localRootLen = strlen(*localRoot) + 1;
    foreach(*p in *localPaths) {
        # use substr to chop off first *localRootLen from the absolute
        # path of the file - to get the next level of directory.
        *p1 = substr(*p, *localRootLen, strlen(*p));
        
        # Concatinate *p1 to the list *cs by adding it as its first element
        *cs = cons(*p1, *cs);
        
    }
    *cs;
}

# Form a list of directories extracted from the list filePaths
# filePaths is the array or list of all the files under the localRoot
getCollections(*filePaths) {
    *cs = list();
    foreach(*p in *filePaths) {
        # Trim everything to the right of "/" to get the
        # directory name.
        *p2 = trimr(*p, "/");
        
        # Get the directory *p2 is not in the list *cs, add it as the 1st element
        if(!contains(*cs, *p2) && *p != *p2) {
            *cs = cons(*p2, *cs);
            #writeLine("stdout", ">>>>>> cs = *cs \n");
        }
    }
    *cs;
}

# Function uploadFiles takes in an array listing of files *localPaths, and copies the files
# to the given location *coll, in the grid. It creates a colelction if it doesn't exist.

uploadFiles: input string * input list string * input string -> integer
uploadFiles(*localRoot, *localPaths, *coll) {
    *fs = getFiles(*localRoot, *localPaths);
    *cs = getCollections(*fs);
    createCollections(*coll, *cs);
    for(*i=0;*i<size(*fs);*i=*i+1) {
        *obj = elem(*fs,*i);
        *lf = elem(*localPaths,*i);
        #writeLine("stdout", "D: uploading *lf --> *coll/*obj");
        msiDataObjPut("*coll/*obj", "demoResc", "*lf++++forceFlag=", *status);       
    }
}
