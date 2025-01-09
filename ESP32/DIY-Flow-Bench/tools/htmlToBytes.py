import os
import zlib
import json
from pathlib import Path

def convert_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Compress content    
    compressed = zlib.compress(content.encode())
    
    # Convert to byte array
    bytes_str = ','.join([f"0x{b:02x}" for b in compressed])
    
    # Get filename without extension
    basename = os.path.splitext(os.path.basename(filepath))[0]
    
    # Get file extension without the dot
    extension = os.path.splitext(filepath)[1][1:]
    
    return f"""
// {filepath}
const uint8_t {basename}_{extension}[] PROGMEM = {{{bytes_str}}};
const uint16_t {basename}_{extension}_len = {len(compressed)};
"""

def main():
    output = """#pragma once
#include <pgmspace.h>

// Auto-generated file - do not edit
"""
    
    # Process all files in html dir
    src_dir = Path(__file__).parent.parent / 'html'
    for file in os.listdir(src_dir):
        if file.endswith(('.html', '.js', '.css')):
            filepath = os.path.join(src_dir, file)
            output += convert_file(filepath)

    # Write header file
    output_path = Path(__file__).parent.parent / 'htmldata.h'
    with open(output_path, 'w') as f:
        f.write(output)

if __name__ == "__main__":
    main()