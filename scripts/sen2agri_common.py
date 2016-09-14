#!/usr/bin/python

import os
import datetime
import pipes
import subprocess

#name:      The name of the step as it will be dispayed on screen
#args:      The external process that must be invoked and it arguments
#kwargs:    The list containing the arguments with default values.
#           Possible values are:
#               outf - the file where the output is redirected to (default: "")
#               skip - if True then this step is not executed (default: False)
# rmfiles - the list of files to remove after the execution of the process
# ends (default: [])


def executeStep(name, *args, **kwargs):
    # Check if the output should be redirected to a file
    outf = kwargs.get("outf", "")
    skip = kwargs.get("skip", False)
    rmfiles = kwargs.get("rmfiles", [])
    retry = kwargs.get("retry", False)

    # Get the start date
    startTime = datetime.datetime.now()

    #Check if the step should be skiped
    if skip:
        print "Skipping " + name + " at " + str(startTime)
        return

    retries = 5 if retry else 1
    while retries > 0:
        retries -= 1

        # Print start message
        print "Executing " + name + " at " + str(startTime)

        # Build the command line and print it to the output
        args = map(str, args)
        cmdLine = " ".join(map(pipes.quote, args))
        if len(outf):
            cmdLine = cmdLine + " > " + pipes.quote(outf)
        print cmdLine

        #invoke the external process
        if len(outf):
            fil = open(outf, "w")
            result = subprocess.call(args, stdout=fil)
        else:
            result = subprocess.call(args)

        #Get the end time
        endTime = datetime.datetime.now()

        # Check for errors
        if result != 0:
             print "Error running " + name + " at " + str(endTime) + ". The call returned " + str(result)
             if retries == 0:
                raise Exception("Error running " + name, result)

        # Remove intermediate files if needed
        for fil in rmfiles:
            os.remove(fil)

        # Print end message
        if result == 0:
            print name + " done at " + str(endTime) + ". Duration: " + str(endTime - startTime)
            break
#end executeStep
