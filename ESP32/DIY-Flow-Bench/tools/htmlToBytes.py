import os
import zlib
import gzip
from pathlib import Path

# Compress uzing ZLIB - we will later decompress and serve the content
def convert_to_byte_array(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    compressed = zlib.compress(content.encode())
    bytes_str = ','.join([f"0x{b:02x}" for b in compressed])
    basename = filepath.stem
    extension = filepath.suffix[1:]
    
    return f"""
// {filepath.relative_to(Path(__file__).parent.parent)}
const uint8_t {basename}_{extension}[] PROGMEM = {{{bytes_str}}};
const uint16_t {basename}_{extension}_len = {len(compressed)};
"""

# Compress using GZIP - we will later serve the content directly to the browser
def convert_to_gzip(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    compressed = gzip.compress(content.encode())
    bytes_str = ','.join([f"0x{b:02x}" for b in compressed])
    basename = filepath.stem
    extension = filepath.suffix[1:]
    
    return f"""
// {filepath.relative_to(Path(__file__).parent.parent)}
const uint8_t {basename}_{extension}[] PROGMEM = {{{bytes_str}}};
const uint16_t {basename}_{extension}_len = {len(compressed)};
"""

def main():
    src_dir = Path(__file__).parent.parent / 'html'
    
    header_content = """#pragma once
#include <pgmspace.h>

// Auto-generated file - do not edit
"""
    
    for file in src_dir.glob('*.*'):
        if file.suffix.lower() in ['.js', '.css']:
            header_content += convert_to_gzip(file)
        elif file.suffix.lower() == '.html':
            header_content += convert_to_byte_array(file)
    
    header_file = Path(__file__).parent.parent / 'htmldata.h'
    header_file.write_text(header_content)

if __name__ == "__main__":
    main()