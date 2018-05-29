import subprocess
import sys

def to_str(ss):
    return " ".join(ss)

def run_local_command(args, putMsg=False):
    '''
    run a command at localhost.
    args :: [str] - command line arguments.
                    The head item must be full-path executable.
    putMsg :: bool - put debug message.
    return :: str  - standard output of the command.
    '''

    if putMsg:
        print "run_command:", to_str(args)
    p = subprocess.Popen(args, stdout=subprocess.PIPE,
                         stderr=sys.stderr, close_fds=True)
    f = p.stdout
    s = f.read().strip()
    ret = p.wait()
    print 'ret', ret
#    if ret != 0:
#        raise Exception("command error %s %d\n" % (args, ret))
    if putMsg:
        print "run_command_result:", s
    return s

#print run_local_command(['/usr/bin/lsof', '-i', 'TCP:10200'])
print run_local_command(sys.argv[1:])

