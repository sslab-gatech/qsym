#!/usr/bin/python
#
# @ORIGINAL_AUTHOR: Evgeny Volodarsky
#
# This script contain functions taken from Scripts/remote-builder.py
#

import subprocess
import os
import sys
import re
import argparse
import shlex
import logging

logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.WARNING)
ReturnCode = 0


def SetupOptionParser():
    """
    Setup parser for command line arguments
    """
    parser = argparse.ArgumentParser(description='Launch android apps with Pin.')
    parser.add_argument("--device-id", dest="device_id", action="store", default=None,
        help="ID of target android device connected to the machine. The default is the first device in adb devices.")
    parser.add_argument("--remote-dir", dest="remote_dir", action="store", default=None,
        help="Remote directory to test in on a remote device connected to the machine.")
    parser.add_argument("--and-install", dest="and_install", action="store", default=os.path.join(os.getcwd().rsplit(os.sep,3)[0],'android-install.tar.gz'),
        help="Path to android-install.tar.gz file.")
    parser.add_argument("--busybox-path", dest="busybox_path", action="store", default='/data/busybox/busybox',
        help="Path to busybox executable.")
    parser.add_argument("--isa", dest="isa", action="store", default="ia32",
        help="Instruction Set Architecture ( ia32 / intel64 ).")
    parser.add_argument("--app", dest="app", action="store", default=None,
        help="Namespace of the application you want to instrument, e.g. app.name.space")
    parser.add_argument("--app-path", dest="app_path", action="store", default=None,
        help="Full path to application you want to instrument, e.g. /path/to/app/app.name.space (usually /data/data/app.name.space).")
    parser.add_argument("--main-activity", dest="main_activity", action="store", default=None,
        help="Name of the application activity to be launched.")
    parser.add_argument("--pin-cmd", dest="pin_cmd", action="store", default="",
        help="Pin command line with all needed options including pintool and its options, excluding imstrumented app.")
    parser.add_argument("--verbose", dest="verbose", action="store_true",
        help="Use this option to see extra output.")

    return parser

def GetAndroidDeviceID():
    """
    @return:   A device ID of a device that is connected to the machine.
    @rtype     string.
    """

    try:
        out = RunCommand('adb devices')
        rx = re.compile(r'(\S+)\s+device$', re.MULTILINE)
        dname = rx.search(out).group(1)

    except OSError, e:
        logging.error("Execution failed:" + str(e))
        logging.error("\tcommand: " + cmd)
        ReturnCode = 1

    return dname

def RunCommand(cmd):
    """
    Execute a shell command and wait for it to complete.  If the command fails, an error is printed
    and E{ReturnCode} is set to non-zero.

    @param cmd:     The shell command to run.
    @type cmd:      string.

    @return:        Shell command output.
    @rtype          string.
    """

    global ReturnCode
    if ReturnCode != 0:
        logging.error(">>> command was not executed due to previous failures")
        logging.error(">>> " + cmd)
        return ""

    logging.info(">>> " + cmd)

    # Flush ensures that any I/O printed by this script appears before any I/O from the command
    # we're about to execute.
    sys.stdout.flush()

    try:
        sub = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        out, err = sub.communicate()
        ret = sub.wait()
    except OSError, e:
        logging.error("Execution failed:" + str(e))
        logging.error("\tcommand: " + cmd)
        ReturnCode = 1

    if ret != 0:
        logging.error("Execution failed with return code " + str(ret))
        logging.error("\tcommand: " + cmd)
        ReturnCode = ret

    return out

def ParseCmd(cmdLine):
    """
    Parse Pin command line

    @param cmdLine:     Pin command line.
    @type cmdLine:      string.

    @return:   Pin / tool options.
    @rtype     dictionary.
    """
    options = {}
    args = shlex.split(cmdLine)
    # Parse command line and store all options in dictionary
    for knob,value in zip(args, args[1:]+["--"]):
        if knob.startswith('-'):
            if value.startswith('-'):
                options[knob] = True
            else:
                options[knob] = value
    return options

def CreateLaunchScript(params, pinCmd):
    """
    Create script, that will launch instrumented app with Pin on the device

    @param params:     Parameters.
    @type params:      ScriptData.
    @param pinCmd:     Pin command line.
    @type pinCmd:      string.
    """
    if pinCmd == None or pinCmd == "":
        logging.error('Pin command line not specified. Use --pin-cmd option to specify it.')
        ReturnCode = 1
        return
    else:
        # Parse pin command line and store all pin options and tool options in "pinOptions" dictionary
        pinOptions = ParseCmd(pinCmd)

    # Make sure that pin command line does not contain instrumented app
    if ' -- ' in pinCmd:
        logging.error('Pin command line specified with --pin-cmd option should not contain instrumented app.')
        ReturnCode = 1
        return

    pin_exec = pinCmd[: pinCmd.find(' ')]
    # If full path to pin executable not given
    if not pin_exec.startswith('/'):
        pin_exec = os.path.join(params.remote_dir,pin_exec)

    # Replace Pin executable in given Pin command line
    androidPinCmd = pin_exec + pinCmd[pinCmd.find(' '):]

    # If logfile name and path doesn't exist in pinOptions - define and add it to command line.
    if '-logfile' not in pinOptions:
        logpath = os.path.join(params.app_path,'pin.log')
        # Replace Pin executable in given Pin command line and add logfile
        androidPinCmd = pin_exec + " -logfile " + logpath + pinCmd[pinCmd.find(' '):]
        logging.info('Pin log file placed at: %s', logpath)

    # Get tool name and path from pinOptions
    if '-t64' in pinOptions:
        params.tool64path = pinOptions['-t64']
        tool64dir,tool64 = os.path.split(params.tool64path)
        params.obj_dir64 = os.path.join(params.remote_dir,'obj-intel64')
        # Replace 64-bit pintool path in given Pin command line
        androidPinCmd = androidPinCmd.replace(params.tool64path, os.path.join(params.obj_dir64,tool64))
        if '-t' in pinOptions:
            params.toolpath = pinOptions['-t']
            tooldir,tool = os.path.split(params.toolpath)
            params.obj_dir = os.path.join(params.remote_dir,'obj-ia32')
            # Replace 32-bit pintool path in given Pin command line
            androidPinCmd = androidPinCmd.replace(params.toolpath, os.path.join(params.obj_dir,tool))
        else:
            logging.info('32-bit tool not specified. If 32-bit application given, Pin will run it without applying any tool')
    elif '-t' in pinOptions:
        params.toolpath = pinOptions['-t']
        tooldir,tool = os.path.split(params.toolpath)
        params.obj_dir = os.path.join(params.remote_dir,'obj-'+params.isa)
        # Replace pintool path in given Pin command line
        androidPinCmd = androidPinCmd.replace(params.toolpath, os.path.join(params.obj_dir,tool))
    else:
        logging.info('Pintool not specified. Pin will run the application without applying any tool.')

    launch_script = open(params.launch_script_name, 'w')
    params.script = "#!/system/bin/sh\n"
    params.script += "echo Running $* with pin launcher > " + os.path.join(params.app_path,'launch.log') + "\n"
    params.script += "exec " + androidPinCmd + " -- $*\n"
    launch_script.write(params.script);
    launch_script.close()
    params.launch_script_path = os.path.join(os.getcwd(),launch_script.name)
    os.chmod(params.launch_script_path, 511)

def CreateCommandsList(params):
    """
    Create list of commands to execute

    @param params:     Parameters.
    @type params:      ScriptData.

    @return:   List of commands.
    @rtype     list of strings.
    """

    property_name = 'wrap.' + params.app
    # If the property name length exceeds the limit (PROP_NAME_MAX = 31), truncate it.
    if len(property_name) > 31:
        property_name = property_name[0:31]

    cmds = [
            # Create the remote directory if it does not exist
            'adb -s %s shell mkdir -p %s' % (params.device_id, params.remote_dir),
            # Push android-install.tar.gz to the device
            'adb -s %s push %s %s' % (params.device_id, params.and_install, params.remote_dir),
            # Untar the android-install.tar.gz
            'adb -s %s shell "cd %s; %s tar -zpxvf android-install.tar.gz"' % (params.device_id, params.remote_dir, params.busybox),
           ]
    if params.obj_dir != "":
           cmds.extend([
            # Create obj-ia32/obj-intel64 folder if it does not exist
            'adb -s %s shell mkdir -p %s' % (params.device_id, params.obj_dir),
            # Push pintool to the device
            'adb -s %s push %s %s' % (params.device_id, params.toolpath, params.obj_dir),
           ])
    if params.obj_dir64 != "":
           cmds.extend([
            # Create obj-intel64 folder if it does not exist
            'adb -s %s shell mkdir -p %s' % (params.device_id, params.obj_dir64),
            # Push 64-bit pintool to the device
            'adb -s %s push %s %s' % (params.device_id, params.tool64path, params.obj_dir64),
           ])
    cmds.extend([
            # Push script, that will launch app with Pin to the device
            'adb -s %s push %s %s' % (params.device_id, params.launch_script_name, params.remote_dir),
            # Set android system property to launch instrumented app with the script
            'adb -s %s shell su -c "setprop %s \'logwrapper %s\'"' % (params.device_id, property_name, os.path.join(params.remote_dir,params.launch_script_name)),
            # Stop the instrumented app (if it is running)
            'adb -s %s shell su -c "am force-stop %s"' % (params.device_id, params.app),
            # Start the instrumented app
            'adb -s %s shell su -c "am start -n %s/.%s"' % (params.device_id, params.app,  params.main_activity)
           ])
    return cmds

def PrintDebugInfo(params, cmd_list):
    """
    Print debug info

    @param params:     Parameters.
    @type params:      ScriptData.
    @param cmd_list:   List of commands.
    @type cmd_list:    list of strings.
    """
    logging.debug('Remote working directory:       %s', params.remote_dir)
    logging.debug('Path to android-install.tar.gz: %s', params.and_install)
    logging.debug('Path to busybox:                %s', params.busybox)
    logging.debug('Path to pintool on host:        %s', params.toolpath)
    logging.debug('Path to application:            %s', params.app_path)

    c_list = 'List of commands:'
    for line in cmd_list: c_list += '\n           ' + line
    logging.debug(c_list)

    logging.debug('Path to launch script:          %s', params.launch_script_path)
    logging.debug('Launch script:\n%s', params.script)

class ScriptData:
    device_id = ""
    remote_dir = ""
    isa = ""
    obj_dir = ""
    obj_dir64 = ""
    and_install = ""
    busybox = ""
    toolpath = ""
    tool64path = ""
    app = ""
    main_activity = ""
    script = ""
    launch_script_path = ""
    launch_script_name = "launchpin.sh"

def SetParameters(args):
    params = ScriptData()

    if args.device_id == None or args.device_id == "":
        params.device_id = GetAndroidDeviceID()
    else:
        params.device_id = args.device_id
    logging.info('Android device id: %s', params.device_id)

    if args.remote_dir == None or args.remote_dir == "":
        logging.error('Please provide path to working directory, that will be created on remote device. Use --remote-dir option.')
        ReturnCode = 1
        return params
    else:
        params.remote_dir = args.remote_dir

    params.isa = args.isa
    params.and_install = args.and_install
    # Check whether android-install.tar.gz exists
    if not os.path.isfile(params.and_install):
        logging.error('Please provide correct path to android-install.tar.gz file (use --and-install option).')
        ReturnCode = 1
        return params

    params.busybox = args.busybox_path
    # Check whether busybox exists on the device
    output = subprocess.check_output("adb shell ls %s"%params.busybox, stderr=subprocess.STDOUT, shell=True)
    if 'No such file or directory' in output:
        logging.error('Please provide correct path to busybox (use --busybox-path option).')
        ReturnCode = 1
        return params

    if args.app_path == None or args.app_path == "":
        if args.app == None or args.app == "":
            logging.error('Application not specified. Use --app or --app-path option to specify app you want to instrument.')
            ReturnCode = 1
            return params
        else:
            params.app = args.app
            params.app_path = os.path.join('/','data','data',params.app)
            logging.info('Application directory not specified. It was set to default: "%s".'%params.app_path)
    else:
        params.app_path = args.app_path
        app_dir,params.app = os.path.split(params.app_path)
        if args.app != None and args.app != "" and args.app != params.app:
            logging.error('Application specified with --app option inconsistent with application specified with --app-path option.')
            ReturnCode = 1
            return params

    if args.main_activity == None or args.main_activity == "":
        logging.error('Application Activity name not specified. Use --main-activity option to specify it.')
        ReturnCode = 1
        return params
    else:
        params.main_activity = args.main_activity

    return params


def Main(argv):
    parser = SetupOptionParser();
    args = parser.parse_args()

    # If --verbose option was used, display INFO and DEBUG messages
    if args.verbose == True:
        logging.root.setLevel(logging.DEBUG)

    # Set script parameters
    params = SetParameters(args)
    if ReturnCode:
        return ReturnCode

    # Create script, that will launch instrumented app with Pin on the device
    CreateLaunchScript(params, args.pin_cmd)
    if ReturnCode:
        return ReturnCode

    # Create list of commadns to run
    cmd_list = CreateCommandsList(params)

    # Print debug info if needed
    PrintDebugInfo(params, cmd_list)

    cmd = ";".join(cmd_list)
    RunCommand(cmd)
    return ReturnCode

if __name__=="__main__":
    sys.exit(Main(sys.argv))