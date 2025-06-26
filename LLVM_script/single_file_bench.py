import os
import sys
from pathlib import Path

PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
BENCH_RAW_FOLDER = os.path.join(PROJECT_DIR, "benchmark_raw/")
SINGLE_SOURCE_FOLDER = os.path.join(PROJECT_DIR, "single_file_data/src")
SINGLE_INCLUDE_FOLDER = os.path.join(PROJECT_DIR, "single_file_data/includes")
STATIC_SOURCE_FOLDER = os.path.join(PROJECT_DIR, "static_file_data/src")
STATIC_INCLUDE_FOLDER = os.path.join(PROJECT_DIR, "static_file_data/includes")
COMPILE_SOURCE_FOLDER = os.path.join(PROJECT_DIR, "src/")
COMPILE_INCLUDES_FOLDER = os.path.join(PROJECT_DIR, "includes/")
BUILD_FOLDER = os.path.join(PROJECT_DIR, "build/")
OUTPUT_BIN_FOLDER = os.path.join(PROJECT_DIR, "output_bin/")



def find_and_cp_src_files_single():
    for item in list(Path(BENCH_RAW_FOLDER).rglob("*.c")):
        os.system("cp " + str(item) + " " + SINGLE_SOURCE_FOLDER)
    for item in list(Path(BENCH_RAW_FOLDER).rglob("*.h")):
        os.system("cp " + str(item) + " " + SINGLE_INCLUDE_FOLDER)

def find_and_cp_src_files_static():
    for item in list(Path(BENCH_RAW_FOLDER).rglob("*.c")):
        os.system("cp " + str(item) + " " + STATIC_SOURCE_FOLDER)
    for item in list(Path(BENCH_RAW_FOLDER).rglob("*.h")):
        os.system("cp " + str(item) + " " + STATIC_INCLUDE_FOLDER)

def cp_single_files_to_src():
    compile_source_list = []
    for item in list(Path(SINGLE_SOURCE_FOLDER).rglob("*.c")):
        os.system("cp " + str(item) + " " + COMPILE_SOURCE_FOLDER)
        compile_source_list.append(os.path.join(COMPILE_SOURCE_FOLDER, os.path.basename(item)))
    return compile_source_list

def cp_single_file_headers_to_includes():
    header_list = []
    for item in list(Path(SINGLE_INCLUDE_FOLDER).rglob("*.h")):
        os.system("cp " + str(item) + " " + COMPILE_INCLUDES_FOLDER)
        header_list.append(os.path.join(COMPILE_INCLUDES_FOLDER, os.path.basename(item)))
    return header_list




def rm_single_files_from_compiling_folder(compile_source_list):
    for file in compile_source_list:
        os.system("rm " + file)

def clear_ll():
    os.system("rm " + BUILD_FOLDER + "/*")
def clear_bin():
    os.system("rm " + OUTPUT_BIN_FOLDER + "/*")

if __name__ == "__main__":

    if sys.argv[1] == "static":
        find_and_cp_src_files_static()
        
    else:
        src_list =  cp_single_files_to_src()
        header_list = cp_single_file_headers_to_includes()
        os.system("python3 build_prj.py ../dyn_pass_prj/build/main_pass  " + OUTPUT_BIN_FOLDER)
        rm_single_files_from_compiling_folder(src_list)
        rm_single_files_from_compiling_folder(header_list)
        clear_ll()
        clear_bin()
