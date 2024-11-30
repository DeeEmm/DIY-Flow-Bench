Import("env")
import os
import re
import shutil

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
        match = re.search(r'"RELEASE" :\s+"(.+)"', content)
        if match:
            return match.group(1)
        else:
            return None


def extract_build():
    config_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/version.json")
    with open(config_path, "r") as file:
        content = file.read()
        match = re.search(r'"BUILD_NUMBER" :\s+"(.+)"', content)
        if match:
            return match.group(1)
        else:
            return None
        

def extract_gui_build():
    config_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/version.json")
    with open(config_path, "r") as file:
        content = file.read()
        match = re.search(r'"GUI_BUILD_NUMBER" :\s+"(.+)"', content)
        if match:
            return match.group(1)
        else:
            return None
        

def delete_files_in_directory(directory_path):
   try:
     files = os.listdir(directory_path)
     for file in files:
       file_path = os.path.join(directory_path, file)
       if os.path.isfile(file_path):
         os.remove(file_path)
     print("All files deleted successfully.")
   except OSError:
     print("Error occurred while deleting files.")
        

def merge_bin(source, target, env):

    print("Creating merged binary...")

    gui_build = extract_gui_build()
    build = extract_build()
    release = extract_release()

    release_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/release/")
    project_path = env.subst("$PROJECT_DIR/ESP32/DIY-Flow-Bench/")
    bootloader_path = ".pio/build/esp32dev/bootloader.bin"
    partitions_path = ".pio/build/esp32dev/partitions.bin"
    firmware_path = ".pio/build/esp32dev/firmware.bin"

    merged_file = os.path.join(release_path, f"{release}_{build}_install.bin")
    update_file = os.path.join(release_path, f"{release}_{build}_update.bin")

    GUI_file = os.path.join(project_path, f"data/index.html")
    GUI_output_file = os.path.join(project_path, f"data/index.html")
    releases_directory =  os.path.join(project_path, f"release/")
    data_directory =  os.path.join(project_path, f"data/")
    GUI_release = os.path.join(release_path, f"{release}_{gui_build}_index.html")

    # clear the release directory
    delete_files_in_directory(releases_directory)

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
            # + flash_images
        )
    )

    # env.Execute(f'esptool.py --chip ESP32 merge_bin -o "%s" % {merged_file} --flash_mode dio --flash_size 4MB 0x1000 {bootloader_path} 0x8000 {partitions_path} 0x10000 {firmware_path}')

    # Create the update.bin file
    shutil.copy(".pio/build/esp32dev/firmware.bin", update_file)

    # create the index.html file from minified gul[p file]
    shutil.copy(GUI_file, GUI_release)

    # Remove minified gulp file from data directory (prevents users using file without version name)
    # try: os.remove(GUI_output_file)
    # except FileNotFoundError: print(f"File '{GUI_output_file}' not found.")
    delete_files_in_directory(data_directory)

# Add a post action that runs esptoolpy to merge available flash images
env.AddPostAction(APP_BIN , merge_bin)
