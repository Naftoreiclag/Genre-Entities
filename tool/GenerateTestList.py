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
from Common import get_project_name

proj_name = get_project_name()

luasListVector = '/*### LUA TESTS LIST ###*/'
cppsListVector = '/*### TESTS LIST ###*/'
cppsFwdDeclVector = '/*### TESTS FWD ###*/'
boilerplateFilename = 'TestsBoilerplate.hpp'
outputFilename = '../src/' + proj_name + '/test/Tests.hpp'
cppSourcesDir = '../src/' + proj_name + '/test/'
luaSourcesDir = '../run/test/tests/'

cppsFiles, _, __ = indexFiles(cppSourcesDir, ['.cpp'], [], True)
luasFiles, _, __ = indexFiles(luaSourcesDir, ['.lua'], [], True)

import re

patternCppTestAnnot = re.compile('\s*//@Test\s+(.*)')
patternCppFunc = re.compile('\s*void\s+(\S+)\s*\(\s*\).*')
patternLuaNameAnnot = re.compile('--@Name\s+(.*)')

cppFuncs = {}

for item in cppsFiles:
    filename = cppSourcesDir + item
    with open(filename, 'r') as sourceFile:
        annotName = None
        for line in sourceFile:
            annotation = patternCppTestAnnot.match(line)
            if annotation:
                annotName = annotation.group(1)
                continue
            if annotName is not None:
                match = patternCppFunc.match(line)
                if match:
                    funcName = match.group(1)
                    if funcName in cppFuncs:
                        print('WARNING: Function ' + funcName + ' duplicated')
                    cppFuncs[funcName] = annotName
                annotName = None

luaFiles = {}
for item in luasFiles:
    filename = luaSourcesDir + item
    with open(filename, 'r') as sourceFile:
        name = None
        for line in sourceFile:
            nameComment = patternLuaNameAnnot.match(line)
            if nameComment:
                name = nameComment.group(1)
                break
        if not name:
            print('WARNING: Lua file ' + filename + ' has no name!')
            name = 'Unnamed Test (' + item + ')'
        luaFiles[item] = name


cppsList = []
cppsFwdDecls = []
luasList = []

cppFuncNamesSorted = []
for funcName in cppFuncs:
    cppFuncNamesSorted.append(funcName)
cppFuncNamesSorted.sort()

luaFilenamesSorted = []
for filename in luaFiles:
    luaFilenamesSorted.append(filename)
luaFilenamesSorted.sort()

for funcName in cppFuncNamesSorted:
    print('\tC++ Test: ' + funcName)
    cppsFwdDecls.append('void ' + funcName + '();')
    cppsList.append('    {"' + cppFuncs[funcName] + '", ' + funcName + '},')
for filename in luaFilenamesSorted:
    print('\tLua Test: ' + filename)
    luasList.append('    {"' + luaFiles[filename] + '", "' + filename + '"},')

replacements = {}
replacements[cppsListVector] = cppsList
replacements[cppsFwdDeclVector] = cppsFwdDecls
replacements[luasListVector] = luasList
writeWithReplacements(boilerplateFilename, outputFilename, replacements)