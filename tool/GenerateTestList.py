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
from Common import indexFiles
from Common import writeWithReplacements

testsListVector = '/*### TESTS LIST ###*/'
testsFwdDeclVector = '/*### TESTS FWD ###*/'
boilerplateFilename = 'TestsBoilerplate.hpp'
outputFilename = '../src/pegr/test/Tests.hpp'
testSourcesDir = '../src/pegr/test/'

testCpps, _, __ = indexFiles(testSourcesDir, ['.cpp'], [], True)

import re

testAnnotationPattern = re.compile('\s*//@Test\s+(.*)')
testFuncPattern = re.compile('\s*bool\s+(\S+)\s*\(\s*\).*')

funcs = {}

for item in testCpps:
    filename = testSourcesDir + item
    with open(filename, 'r') as sourceFile:
        annotName = None
        for line in sourceFile:
            annotation = testAnnotationPattern.match(line)
            if annotation:
                annotName = annotation.group(1)
                continue
            if annotName is not None:
                match = testFuncPattern.match(line)
                if match:
                    funcName = match.group(1)
                    print('\t' + funcName)
                    if funcName in funcs:
                        print('WARNING: That function is duplicated')
                    funcs[funcName] = annotName
                annotName = None

testsList = []
testsFwdDecls = []

for funcName in funcs:
    testsFwdDecls.append('bool ' + funcName + '();')
    testsList.append('    {"' + funcs[funcName] + '", ' + funcName + '},')

replacements = {}
replacements[testsListVector] = testsList
replacements[testsFwdDeclVector] = testsFwdDecls
writeWithReplacements(boilerplateFilename, outputFilename, replacements)