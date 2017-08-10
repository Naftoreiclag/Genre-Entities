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

import xml.etree.ElementTree as ET
import os
from Common import get_project_name

proj_name = get_project_name()

from Common import indexFiles
sourceList, _, __ = \
    indexFiles('../src/' + proj_name + '/', ['.cpp', '.hpp'], ['deprecated/'])

projectFilename = '../ide/Codelite/Codelite.project'
sourceRootRelativePath = '../../src/' + proj_name + '/'

projectEtree = ET.parse(projectFilename)
projectRoot = projectEtree.getroot()
virtualDirs = projectRoot.findall('VirtualDirectory')

myprojectVirtualDir = None
for virtualDir in virtualDirs:
    if virtualDir.get('Name') == proj_name:
        myprojectVirtualDir = virtualDir
        break
if not myprojectVirtualDir:
    myprojectVirtualDir = ET.SubElement(projectRoot, 'VirtualDirectory')

myprojectVirtualDir.clear()
myprojectVirtualDir.set('Name', proj_name)

for sourcePath in sourceList:
    
    def decomposePath(path):
        '''
        Converts a path into a list of directory names leading up to the tail
        (filename) and the filename.
        '''
        path, filename = os.path.split(path)
        decomposed = []
        while True:
            head, tail = os.path.split(path)
            if tail:
                decomposed.append(tail)
                path = head
            else:
                break
        decomposed.reverse()
        return decomposed
    
    path = decomposePath(sourcePath)
    
    parentElem = myprojectVirtualDir
    for dirName in path:
        parentChildren = parentElem.findall('VirtualDirectory')
        
        foundChild = None
        for childElem in parentChildren:
            if childElem.get('Name') == dirName:
                foundChild = childElem
                break
        if foundChild:
            parentElem = foundChild
        else:
            parentElem = ET.SubElement(parentElem, 'VirtualDirectory')
            parentElem.set('Name', dirName)

    sourceElem = ET.SubElement(parentElem, 'File')
    sourceElem.set('Name', \
        os.path.join(sourceRootRelativePath, sourcePath))

projectEtree.write(projectFilename)
