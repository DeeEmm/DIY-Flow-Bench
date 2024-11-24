import json
import sys
import os
from SCons.Script import Import

Import("env")

print("Reading version.json")

# read json file into var
file_path = 'ESP32/DIY-Flow-Bench/version.json'
with open(file_path) as file_data:
    json_data = json.load(file_data)

# Iterate through JSON vars and add them to the build environment
print("Adding version data to build environment")
for key, value in json_data.items():
    env.Append(CPPDEFINES=[f'{key}=\\"{value}\\"'])
    print(f'{key}="{value}"')