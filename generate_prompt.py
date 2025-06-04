import os
import shutil
import json
import time
from pathlib import Path
from tqdm import tqdm

# 基础路径配置

BASE_RESULTS_DIR = os.path.join(os.getcwd(), "Results")
PROMTP_RESULTS_DIR = os.path.join(os.getcwd(), "PromptData_Results")
PROJECTS = [
    "curl", "gcc", "git", "h2o", "libgit2", 
    "linux", "mysql", "protobuf", "redis", "tmux"
]

def get_project_paths(project_name: str) -> tuple[str, str, str]:
    """
    获取项目相关的所有路径
    返回: (项目源文件夹, txt输出文件夹, json输出文件夹)
    """
    source_dir = os.path.join(BASE_RESULTS_DIR, project_name)
    txt_output_dir = os.path.join(PROMTP_RESULTS_DIR, f"{project_name}_prompts")
    json_output_dir = os.path.join(PROMTP_RESULTS_DIR, f"{project_name}_prompts_json")
    return source_dir, txt_output_dir, json_output_dir

def clean_output_folders(project_name: str):
    """清理指定项目的输出文件夹"""
    _, txt_dir, json_dir = get_project_paths(project_name)
    for folder in [txt_dir, json_dir]:
        if os.path.exists(folder):
            print(f"清理输出文件夹: {folder}")
            shutil.rmtree(folder)
        os.makedirs(folder)
        print(f"文件夹已重新创建: {folder}")

def create_prompt(c_file_path: Path, txt_file_path: Path) -> tuple[str, dict]:
    """
    创建标准化的 prompt 文本和 JSON 格式
    返回: (txt_content, json_content) 的元组
    """
    # 读取 .c 文件内容
    with open(c_file_path, "r", encoding="utf-8") as f:
        c_content = f.read()
    
    # 读取指针信息
    with open(txt_file_path, "r", encoding="utf-8") as f:
        txt_content = f.read()
        
    ptr_info = txt_content.strip().split(",")
    if len(ptr_info) < 7:
        raise ValueError(f"指针信息格式无效: {txt_file_path}")
        
    ptr1, file1, line1, ptr2, file2, line2, label = ptr_info[:7]
    
    result_map = {
        "1": "MUST",
        "2": "MAY", 
        "0": "MUST-NOT"
    }
    expected_result = result_map.get(label, "Unknown")
    
    # 构建问题描述
    question = (
        f"Below is a function in C. Please analyze the alias relationship between:\n"
        f"1. Pointer '{ptr1}' at line {line1}\n"
        f"2. Pointer '{ptr2}' at line {line2}\n\n"
        f"Determine if these two pointers MUST alias (always point to the same location), "
        f"MAY alias (might point to the same location), or "
        f"MUST-NOT alias (never point to the same location) after the execution of the function.\n\n"
        f"Program to analyze:\n{c_content}"
    )
    
    # 构建文本格式的 prompt
    txt_prompt = (
        "[ROLE]\n"
        "Pointer analyzer that analyzes whether two pointer in a program alias\n\n"
        "[QUESTION]\n"
        f"{question}\n\n"
        "[ANSWER]\n"
        f"{expected_result}\n"
    )
    
    # 构建 JSON 格式的 prompt
    json_prompt = {
        "ROLE": "Pointer analyzer that analyzes whether two pointer in a program alias",
        "QUESTION": question,
        "ANSWER": expected_result
    }
    
    return txt_prompt, json_prompt

def process_project_files(project_name: str):
    """处理单个项目文件夹中的所有文件对"""
    source_dir, txt_output_dir, json_output_dir = get_project_paths(project_name)
    
    if not os.path.exists(source_dir):
        print(f"错误: 项目目录不存在: {source_dir}")
        return
        
    # 清理输出目录，但不打印清理信息
    clean_output_folders(project_name)
    
    # 获取所有 .c files 并预先过滤掉没有对应txt文件的内容
    c_files = []
    for c_file in Path(source_dir).glob("*.c"):
        if c_file.with_suffix(".txt").exists():
            c_files.append(c_file)
    
    total_files = len(c_files)
    
    # 使用sys.stdout强制刷新，确保进度条正确显示
    import sys
    sys.stdout.flush()
    
    # 配置进度条，使用\r确保在同一行更新
    with tqdm(total=total_files,
             desc=f"处理 {project_name}",
             bar_format='\r{desc}:|{bar:50}|{percentage:3.0f}%',
             ascii=True,
             ncols=80,
             file=sys.stdout,
             miniters=1) as pbar:
        
        for c_file in c_files:
            try:
                txt_prompt, json_prompt = create_prompt(c_file, c_file.with_suffix(".txt"))
                
                # 保存文件但不打印任何信息
                txt_prompt_file = Path(txt_output_dir) / f"{c_file.stem}_prompt.txt"
                with open(txt_prompt_file, "w", encoding="utf-8") as f:
                    f.write(txt_prompt)
                    
                json_prompt_file = Path(json_output_dir) / f"{c_file.stem}_prompt.json"
                with open(json_prompt_file, "w", encoding="utf-8") as f:
                    json.dump(json_prompt, f, indent=2, ensure_ascii=False)
                
            except Exception as e:
                pass
            finally:
                pbar.update(1)
                sys.stdout.flush()  # 强制刷新输出
    
    # 进度条结束后打印换行
    print()

def process_all_projects():
    """处理所有项目文件夹"""
    print("开始批量处理所有项目文件夹")
    
    total_start_time = time.time()
    
    for project in PROJECTS:
        project_start_time = time.time()
        try:
            process_project_files(project)
        except Exception as e:
            print(f"处理项目 {project} 时发生错误: {str(e)}")
        finally:
            project_end_time = time.time()
            print(f"项目 {project} 处理耗时: {project_end_time - project_start_time:.2f} 秒")
            
    total_end_time = time.time()
    print(f"\n所有项目处理完成！总耗时: {total_end_time - total_start_time:.2f} 秒")

if __name__ == "__main__":
    process_all_projects()