# user_actions_pre.py
# This file is part of the DIY FLow Bench Project. https//github.com/DeeEmm/DIY-Flow-Bench
# Author: DeeEmm
import json
import sys
import os
import datetime
import re
import shutil
from SCons.Script import Import
from subprocess import run

Import("env")

# stop the current script execution if called from external script
if env.IsIntegrationDump():
    Return()

print("Pre-Build tasks")

print("Build htmldata.h")

# convert HTML to byte data and store in htmldata.h
run(["python3", "ESP32/DIY-Flow-Bench/tools/htmlToBytes.py"])

release_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/release/")

print("Loading version.json")

# read json file into var
file_path = 'ESP32/DIY-Flow-Bench/version.json'
with open(file_path) as file_data:
    json_data = json.load(file_data)

# increment build and update version.json

# get current date
dtnow = datetime.datetime.now()
year = dtnow.strftime("%y")
month = dtnow.strftime("%m")
date = dtnow.strftime("%d")

# read build No from json
build_num = json_data['BUILD_NUMBER']
release = json_data['RELEASE']
# print(build_num  + "\n")

# get current details and delete files
old_merged_file = os.path.join(release_path, f"{release}_{build_num}_install.bin")
old_update_file = os.path.join(release_path, f"{release}_{build_num}_update.bin")
try:
    os.remove(old_merged_file)
    os.remove(old_update_file)
except OSError:
    print("Error occurred while deleting files.")

# print(old_merged_file)

# get build date info from version.json
bn_year = build_num[0:2]
bn_month =  build_num[2:4]
bn_date =  build_num[4:6]
bn_inc = build_num[6:10]
# print(bn_year  + "\n")
# print(bn_month  + "\n")
# print(bn_date  + "\n")
# print(bn_inc  + "\n")

# check build date incremental count
if bn_year == year and bn_month == month and bn_date == date:
    # we are still on same day lets increment existing build number
    incremental = int(build_num[6:10])
    incremental += 1    
else:
    # it's a new day start from 0001
    incremental = 1

## add preceding zeroes if required.
inc_str = str(incremental).zfill(4)
print("incremental build #: " + inc_str)

# create build number
json_data['BUILD_NUMBER'] = year + month + date + inc_str

# update version.json file
print("Updating version.json")
with open(file_path, 'w') as x:
    json.dump(json_data, x, indent=2)

# Iterate through JSON vars and add them to the build environment
# Build var template items get updated as part of build
print("Adding version data to build environment...")
for key, value in json_data.items():
    env.Append(CPPDEFINES=[f'{key}=\\"{value}\\"'])
    print(f'{key}="{value}"')

