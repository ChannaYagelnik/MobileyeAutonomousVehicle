#!/bin/bash

# Check if enough arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <output_directory> <output_filename>"
    exit 1
fi

# Get the output directory and file name from arguments
output_directory="$1"
output_filename="$2"

# Create the directory if it doesn't exist
mkdir -p "$output_directory"

# Define the JSON content
json_content='{
    "endianness": "little",
    "fields": [
        {
            "size": 32,
            "type": "signed_int",
            "name": "Status"
        },
        {
            "size": 32,
            "type": "signed_int",
            "name": "Code"
        },
        {
            "size": 32,
            "type": "signed_int",
            "name": "Flags"
        }
    ]
}'

# Create the full path for the output JSON file
output_file_path="$output_directory/$output_filename"

# Write the JSON content to the specified file
echo "$json_content" > "$output_file_path"

echo "JSON file created: $output_file_path"