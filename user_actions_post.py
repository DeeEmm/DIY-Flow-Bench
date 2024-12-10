import json
import sys
import os
import datetime
import re
import shutil
from SCons.Script import Import

Import("env")


# Source - https://github.com/platformio/platform-espressif32/issues/1078
# Also ...
# https://github.com/arendst/Tasmota/blob/development/pio-tools/post_esp32.py
# https://github.com/dewenni/ESP_Buderus_KM271/blob/0225e70472b4f6b9568f0901c83c3081cf0be644/platformio_release.py

APP_BIN = "$BUILD_DIR/${PROGNAME}.bin"
MERGED_BIN = "$BUILD_DIR/${PROGNAME}_boot.bin"
BOARD_CONFIG = env.BoardConfig()


def extract_release():
    config_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/version.json")
    with open(config_path, "r") as file:
        content = file.read()
        match = re.search(r'"RELEASE":\s+"(.+)"', content)
        if match:
            return match.group(1)
        else:
            return None


def extract_build():
    config_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/version.json")
    with open(config_path, "r") as file:
        content = file.read()
        match = re.search(r'"BUILD_NUMBER":\s+"(.+)"', content)
        print(match)
        if match:
            return match.group(1)
        else:
            return None
        
        

def extract_json_val(jsonKey):
    config_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/version.json")
    with open(config_path, "r") as file:
        content = file.read()
        pattern = r'"' + jsonKey + '":\s+"(.+)"'
        match = re.search(pattern, content)
        # print(match)
        if match:
            return match.group(1)
        else:
            return None



# DEPRECATED 
# def delete_files_in_directory(directory_path):
#    try:
#      files = os.listdir(directory_path)
#      for file in files:
#        file_path = os.path.join(directory_path, file)
#        if os.path.isfile(file_path):
#          os.remove(file_path)
#      print("All files deleted successfully.")
#    except OSError:
#      print("Error occurred while deleting files.")


# DEPRECATED - Search for wildcard filenames  
# def del_wildcard(wildcard):
#    try:
#      print(wildcard)
#      files = os.listdir(wildcard)
#      for file in files:
#        file_path = os.path.join(wildcard, file)
#        if os.path.isfile(file_path):
#         index = file.find(wildcard)
#         if index > -1:
#            os.remove(file_path)
#      print(file_path + "deleted successfully.")
#    except OSError:
#      print("Error occurred while deleting files.")

     


def after_build(source, target, env):
    
    print("Post-Build tasks")
    print("Creating merged binary...")

    release_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/release/")
    project_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/")
    bootloader_path = ".pio/build/esp32dev/bootloader.bin"
    partitions_path = ".pio/build/esp32dev/partitions.bin"
    firmware_path = ".pio/build/esp32dev/firmware.bin"

    build = extract_json_val("BUILD_NUMBER")
    release = extract_json_val("RELEASE")

    merged_file = os.path.join(release_path, f"{release}_{build}_install.bin")
    update_file = os.path.join(release_path, f"{release}_{build}_update.bin")

    releases_directory =  os.path.join(project_path, f"release/")
    data_directory =  os.path.join(project_path, f"data/")

    release = extract_json_val("RELEASE")
    print(release)

    # Run esptool to merge images into a single binary
    env.Execute(
        " ".join(
            [
                '"%s"' % "$PYTHONEXE",
                "$OBJCOPY",
                "--chip",
                BOARD_CONFIG.get("build.mcu", "esp32"),
                "merge_bin",
                "--fill-flash-size",
                BOARD_CONFIG.get("upload.flash_size", "4MB"),
                "-o",
                '"%s"' % merged_file,
                "0x1000",
                bootloader_path,
                "0x8000",
                partitions_path,
                "0x10000",
                firmware_path
            ]
        )
    )

    # env.Execute(f'esptool.py --chip ESP32 merge_bin -o "%s" % {merged_file} --flash_mode dio --flash_size 4MB 0x1000 {bootloader_path} 0x8000 {partitions_path} 0x10000 {firmware_path}')

    # Create the update.bin file
    shutil.copy(".pio/build/esp32dev/firmware.bin", update_file)

env.AddPostAction(APP_BIN , after_build)
