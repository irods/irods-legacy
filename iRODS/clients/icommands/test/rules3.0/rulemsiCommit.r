acCreateUserF1 {
# This is the policy acCreateUserF1 in the core.re file
 ON ($otherUserName == "anonymous")
 {
   msiCreateUser ::: msiRollback;
   msiCommit;
 }
}
