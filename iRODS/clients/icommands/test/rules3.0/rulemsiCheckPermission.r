acPostProcForPut {
#This micro-service is deprecated.  Instead use msiCheckAccess
# The msiCheckPermission micro-service requires that session variables about
# the user be set, and should be called from Policies that set the S1 session variables
# Input parameter is:
#  Authorization permission 
# No output parameters
   ON (msiCheckPermission("own") == 0) THEN {
     writeLine("stdout","Permission check passed"); }
   ELSE {writeLine("stdout","Permission failed"); }
}
