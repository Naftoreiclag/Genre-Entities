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

def indexFiles(searchPath, allowedFileExts, blacklistedDirs):
    ''' Indexes all of the files located in searchPath.
    
    Only files with the extensions listed in allowedFileExts are included.
    Any files located in the directories (or subdirectories thereof) listed in 
    blacklistedDirs are discluded.
    
    First return is a list of all of the files, relative to searchPath.
    
    Second return is all of the directories relative to searchPath containing
    at least one file.
    
    Third return is a dictionary mapping filenames to full paths relative to
    searchPath.
    '''

    sourceList = []
    fnameToPath = {}
    includeDirList = []

    def addSource(filepath):
        if not any(filepath.startswith(dir) for dir in blacklistedDirs) \
                and any(filepath.endswith(ext) for ext in allowedFileExts):
            sourceList.append(filepath)
            fileParentPath, filename = os.path.split(filepath)
            if fileParentPath and fileParentPath not in includeDirList:
                includeDirList.append(fileParentPath)
            if filename in fnameToPath:
                print('WARNING! File: ' + filename + ' occurs twice!')
                print('\t' + fnameToPath[filename])
                print('\t' + fileParentPath)
            else:
                fnameToPath[filename] = filepath

    def recursiveSearch(prefixStr, location):
        for node in os.listdir(location):
            nodeLocation = os.path.join(location, node)
        
            if os.path.isfile(nodeLocation):
                addSource(prefixStr + str(node))
            else:
                recursiveSearch(prefixStr + str(node) + '/', nodeLocation)

    recursiveSearch('', searchPath)
    sourceList = sorted(sourceList)
    includeDirList = sorted(includeDirList)

    return sourceList, includeDirList, fnameToPath
