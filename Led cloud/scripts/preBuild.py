import os
import sys
import gzip
import shutil
from pathlib import Path

def gzip_file(source_path, target_path):
    """Compress a file using gzip."""
    with open(source_path, 'rb') as src_file:
        with gzip.open(target_path, 'wb') as gz_file:
            gz_file.writelines(src_file)

def process_html_files(data_dir):
    """Process HTML, CSS, and JS files to optimize them for the ESP8266."""
    if not os.path.exists(data_dir):
        print(f"Data directory {data_dir} not found. Skipping file processing.")
        return
    
    print(f"Processing files in {data_dir}...")
    
    # Walk through all files in the data directory
    for root, dirs, files in os.walk(data_dir):
        for filename in files:
            file_path = os.path.join(root, filename)
            
            # Skip already compressed files
            if filename.endswith('.gz'):
                continue
                
            # Check if this is an HTML, CSS or JS file
            if filename.endswith('.html') or filename.endswith('.css') or filename.endswith('.js'):
                print(f"Compressing {filename}")
                compressed_path = file_path + '.gz'
                
                # Create gzipped version of the file
                gzip_file(file_path, compressed_path)
                
                # Get file sizes for comparison
                original_size = os.path.getsize(file_path)
                compressed_size = os.path.getsize(compressed_path)
                
                # Calculate savings
                savings = (1.0 - (compressed_size / original_size)) * 100
                
                print(f"  Original: {original_size} bytes")
                print(f"  Compressed: {compressed_size} bytes")
                print(f"  Savings: {savings:.2f}%")

# Call the main function
data_dir = os.path.join(os.getcwd(), "data")
process_html_files(data_dir)