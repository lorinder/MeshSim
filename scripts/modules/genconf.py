#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys

def apply_template_to_file(fn_in, indir, outdir, kv):
    # Figure out what the output name is and what conversion action to
    # take.
    ext = ""
    j = fn_in.rfind('.')
    if j == -1:
        j = len(fn_in)
    ext = fn_in[j:]
    if ext == ".in":
        fn_out = fn_in[:j]
    elif ext == ".php":
        fn_out = fn_in[:j]
    else:
        # If no match, copy files literally.
        ext = ""
        fn_out = fn_in

    # Make filenames fully qualified
    fn_in = indir + os.sep + fn_in
    fn_out = outdir + os.sep + fn_out

    # Now process file
    if ext == "":
        shutil.copy(fn_in, fn_out)
    elif ext == ".in":
        with open(fn_in, 'r') as fp:
            content = fp.read()
        with open(fn_out, 'w') as fp:
            fp.write(content.format(**kv))
    elif ext == ".php":
        # Find PHP include path
        incdir = os.path.dirname(os.path.realpath(__file__))

        # Construct PHP command line
        args = [ 'php' ]+ [ '-d', "include_path=%s" % (incdir,) ]
        args += [ fn_in ] + ['%s=%s' % (k, v) for k,v in kv.items() ]
        args += [ "_genconf_dir=%s" % \
                  (os.path.dirname(os.path.realpath(sys.argv[0])),) ]
        args += [ "_in_dir=%s" % (os.path.realpath(indir),) ]

        # Run PHP
        fp_out = open(fn_out, 'w')
        ret = subprocess.run(args, stdout=fp_out)
        if ret.returncode != 0:
            sys.stderr.write("Error: PHP error processing \"%s\".\n"
              % (fn_in,))
            return False

    # Success
    return True

def apply_template(indir, outdir, kv):
    """Processes an entire template directory."""

    # Create the output directory
    os.makedirs(outdir, exist_ok=True)

    # Apply template to each file
    for fn_in in os.listdir(indir):
        success = apply_template_to_file(fn_in, indir, outdir, kv)
        if not success:
            return False

    return True
