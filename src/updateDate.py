# Simple script to update all the dates on the .cpp files within the /src folder
# I have intentionally excluded _archive and Wouter folders from this
# Change the Date as needed.

import os

ROOT_FOLDER = os.getcwd() + "/src"  # Set the root folder to "src"

EXCLUDED_FOLDERS = ("_archive", "Wouter")  # Folders to exclude

def replace_line_6_in_cpp_files(root_folder):
    for root, directories, files in os.walk(root_folder):
        # Exclude target folders before iterating through files
        directories[:] = [d for d in directories if d not in EXCLUDED_FOLDERS]

        for filename in files:
            if filename.endswith(".cpp"):  # Modify if needed for other extensions
                file_path = os.path.join(root, filename)
                try:
                    with open(file_path, "r") as file:
                        lines = file.readlines()
                    if len(lines) >= 6:  # Check if file has at least 6 lines
                        # Replace line 6 with the new text
                        lines[5] = "  Date: 24 January 2024\n"
                    else:
                        print(f"File {file_path} has less than 6 lines, skipping.")
                        continue
                    with open(file_path, "w") as file:
                        file.writelines(lines)
                    print(f"Replaced line 6 in {file_path}")
                except Exception as e:
                    print(f"Error modifying file {file_path}: {e}")

replace_line_6_in_cpp_files(ROOT_FOLDER)
