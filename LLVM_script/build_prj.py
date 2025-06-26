import os
import subprocess
import glob
import sys

# 设置项目的路径
PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
SOURCE_DIR = os.path.join(PROJECT_DIR, 'src/')
BUILD_DIR = os.path.join(PROJECT_DIR, 'build/')
INCLUDE_DIR = os.path.join(PROJECT_DIR, 'includes/')

# 确保构建目录存在
os.makedirs(BUILD_DIR, exist_ok=True)

# 找到所有的源文件（.c 文件）
source_files = glob.glob(os.path.join(SOURCE_DIR, "**", "*.c"), recursive=True)

# 编译每个源文件为 LLVM 中间表示（.ll 文件）
def compile_to_ll(source_file):
    # 获取源文件的相对路径（相对于 SOURCE_DIR）
    relative_path = os.path.relpath(source_file, SOURCE_DIR)
    
    # 获取文件名（没有扩展名）
    source_name = os.path.splitext(os.path.basename(source_file))[0]
    
    # 构建在 BUILD_DIR 中的目标文件路径
    # old source compiling does not consider folders
    # source_name = os.path.basename(source_file)
    # ll_file = os.path.join(BUILD_DIR, f"{source_name}.ll")
    ll_file = os.path.join(BUILD_DIR, os.path.dirname(relative_path), f"{source_name}.ll")
    os.makedirs(os.path.dirname(ll_file), exist_ok=True)


    # clang 命令行编译源文件为 .ll 文件，加入包含路径
    clang_command = [
        "clang",  "-emit-llvm", "-O0", "-g3", "-I", INCLUDE_DIR, "-S", source_file, "-o", ll_file
    ]
    print(f"Running command: {' '.join(clang_command)}")
    subprocess.run(clang_command, check=True)
    return ll_file

# 运行 LLVM Pass
def run_main_pass(input_bc_file_path, main_pass_path):
    output_bc_file_path = input_bc_file_path.replace(".bc", "_passed.bc")
    
    opt_command = [
        main_pass_path, input_bc_file_path, output_bc_file_path
    ]
    print(f"Running command: {' '.join(opt_command)}")
    subprocess.run(opt_command, check=True)
    
    return output_bc_file_path

def link_single_to_logged_bc(ll_file, logging_ll):
    output_logged_bc_path = os.path.splitext(ll_file)[0] + "_logged.bc"
    link_command = ["llvm-link", ll_file, logging_ll, "-o", output_logged_bc_path]
    print(f"Running command: {' '.join(link_command)}")
    subprocess.run(link_command, check=True)
    return output_logged_bc_path

def compile_linked_bc_to_binary(merged_bc_file_path, main_pass_path, output_folder_path):
    # do the stabbing
    passed_bc_file_path = run_main_pass(merged_bc_file_path, main_pass_path)
    # compile the stabbed bc to binary
    merged_bc_file_name = os.path.splitext(os.path.basename(passed_bc_file_path))[0]
    output_bin_path = output_folder_path + merged_bc_file_name + "_bin"
    clang_command = ["clang", passed_bc_file_path, "-o", output_bin_path]
    print("Compiling linked bc to binary")
    print(f"Running command: {' '.join(clang_command)}" )
    subprocess.run(clang_command, check=True)
    return output_bin_path
    

def build_ll_and_link():
    print("Building ll files... ")
    ll_files = []
    clexma_logging_file = ""
    for source_file in source_files:
        print("compiling source file: " + source_file)
        ll_file = compile_to_ll(source_file)
        ll_files.append(ll_file)
        if is_clexma_logging_file(ll_file):
            print("clexma logging file found")
            clexma_logging_file = ll_file
    
    for ll_file in ll_files:
        if not is_clexma_logging_file(ll_file):
            link_single_to_logged_bc(ll_file, clexma_logging_file)

def build_ll_link_and_compile(main_pass_path, output_folder):
    print("Building ll files... ")
    ll_files = []
    clexma_logging_file = ""
    for source_file in source_files:
        print("compiling source file: " + source_file)
        ll_file_path = compile_to_ll(source_file)
        ll_files.append(ll_file_path)
        if is_clexma_logging_file(ll_file_path):
            print("clexma logging file found")
            clexma_logging_file = ll_file_path
    
    for ll_file in ll_files:
        if not is_clexma_logging_file(ll_file):
            linked_file_path = link_single_to_logged_bc(ll_file, clexma_logging_file)
            print("file linked: " + linked_file_path)
            compile_linked_bc_to_binary(linked_file_path, main_pass_path, output_folder)

def is_clexma_logging_file(file_name):
    if file_name.find("clexma_logging") >= 0:
        return True
    else:
        return False
    

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Argument insufficient\n Usage: python3 build_prj.py [main pass path] [output folder]")
    
    main_pass_path = sys.argv[1]
    output_folder = sys.argv[2]
    print("main pass path: " + sys.argv[1])
    print("output folder: " + sys.argv[2])
    # build_ll_and_link()
    build_ll_link_and_compile(main_pass_path, output_folder)