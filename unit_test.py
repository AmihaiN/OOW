#!/usr/bin/python

import os
import tempfile
import subprocess
import random
import string

def debug_print(message):
    if type(message) is str:
        print message % globals()
    elif type(message) is list:
        for line in message:
            print "\t" + line % globals()

def random_string(length):
    return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(length))

pid = os.getpid()
test_file = tempfile.NamedTemporaryFile(delete=False)
test_file_name = test_file.name
test_file_dir_name = os.path.dirname(test_file_name)
test_string = random_string(10)

print "useless unit test:"
debug_print(["pid: %(pid)s", "test file: %(test_file_name)s", "test contents: %(test_string)s"])
print

debug_print("Printing test string to file...")
test_file.write(test_string)
test_file.flush()

inode = os.stat(test_file.name).st_ino

debug_print("Removing temp file with inode #%(inode)s...")
os.remove(test_file_name)

debug_print("Running useless_helper...")
retval = subprocess.call(["./useless_helper", str(pid), str(inode)])

if retval != 0:
    debug_print("Helper failed with value %(retval)s")
    os.exit(retval)

test_message = "Checking if file is restored correctly... "
restored_file_name = test_file_dir_name + "/RESTORED"
with open(restored_file_name) as restored:
    if restored.read() == test_string and os.stat(restored_file_name).st_nlink == 1:
        debug_print(test_message + "PASS")
    else:
        debug_print(test_message + "FAIL")
        os.remove(test_file_name)

    os.remove(test_file_dir_name + "/RESTORED")
