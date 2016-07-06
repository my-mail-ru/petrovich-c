#!/usr/bin/env python3

# This script combines all sources into a single '.c' file, so that in can be
# compiled as a part of a project.

import os.path
pjoin = os.path.join

def process_file(fname, dest):
    with open(fname, 'r') as f:
        if '.h' in fname:
            # Skip top comment and include guards
            lines = f.readlines()[6:-1]
        else:
            # Just skip the comment
            lines = f.readlines()[3:]

        for line in lines:
            # Should not happen (no '\n')
            if not line:
                continue
            # Skip project includes
            if line[0] == '#' and '"' in line:
                continue
            # Skip @file doxygen commands
            if '@file' in line:
                continue
            dest.write(line)

def main():
    src_dir = os.path.dirname(__file__)
    dest_file = pjoin(src_dir, 'build', 'petrovich.c')
    with open(dest_file, 'w') as dest:
        dest.write('#include "petrovich.h"\n')
        for name in ['buffer.h', 'utf8.h', 'utf8.c', 'petrovich.c']:
            full_name = pjoin(src_dir, 'lib', name)
            process_file(full_name, dest)

if __name__ == '__main__':
    main()
