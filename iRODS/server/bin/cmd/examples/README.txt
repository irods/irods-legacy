These are example scripts that can be copied from the current
directory, .../server/bin/cmd/examples, up one level to
.../server/bin/cmd and then can be executed via the iexecmd command.
Previously, these were in the server/bin/cmd directory by default, but
it is better practice to have available just those that the
administrator selects.  iexecmd allows authenticated users to run
commands that exist in that directory on the server side (see 
iexecmd -h for more).

The 'irodsctl devtest' copies 'hello' from here to server/bin/cmd for
some of the tests and then removes it.  The test_execstream.py is used
in some Jargon tests and so it too must be moved up to server/bin/cmd
for those tests.  Other than that, these are examples and special
purpose scripts.  Current contents are:

amqprecv.py   python scripts for sending and receiving simple amqp messages;
amqpsend.py   these tested on rabbitm.

getErrorStr   Script to convert error numbers to strings; Since iRODS
              2.5 this is no longer needed as 'ierror' replaces much
              of the functionality.

hello         Simple hello (echo) example.

irodsServerMonPerf   A server monitoring perl script,contributed by
              Jean-Yves.

myWorkFlow    Part of a workflow system, and example program.
tt            A file associated with myWorkFlow.

readRodsLog.py  A python script useful for scanning iRODS log files, 
              contributed by Adil.
readlogs.cfg  Config file associated with readRodsLog.py.

test_execstream.py  A test script used in some Jargon tests, written by Mike C.

univMSSInterface.sh  A template for implementing a universal Mass Storage 
              System driver (Jean-Yves).

