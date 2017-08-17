#   Copyright 2017 James Fong
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import os
from Common import get_project_name

proj_name = get_project_name()

searchPath = '../src/' + proj_name + '/'
includePrefix = proj_name + '/'

from Common import indexFiles

headerFnames, _, _ = indexFiles(searchPath, ['.hpp', '.cpp'], [])

import re

# Also matches some malformed includes and assumes no spaces in the file names, 
# but this case should never come up
includePattern = re.compile('#include\s+["<](\S*)[">]\s*')

for headerFname in headerFnames:
    contents_changed = False
    new_contents = []
    
    with open(searchPath + headerFname, 'r') as headerFile:
        sort_queue = []
        
        def flush_sort_queue():
            if len(sort_queue) > 0:
                sort_queue.sort()
                new_contents += sort_queue
                sort_queue.clear()
                contents_changed = True
        
        for line in headerFile:
            match = includePattern.match(line)
            if match:
                sort_queue.append(line)
            else:
                flush_sort_queue()
                new_contents.append(line)
        flush_sort_queue()
        
    if not contents_changed:
        continue
    
    with open(searchPath + headerFname, 'w') as headerFile:
        for line in new_contents:
            headerFile.write(line)
