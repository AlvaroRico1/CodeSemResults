import os
import shutil
import json
import time
from pathlib import Path
from tqdm import tqdm

# Base path configuration
BASE_RESULTS_DIR = os.path.join(os.getcwd(), "Results")
PROMTP_RESULTS_DIR = os.path.join(os.getcwd(), "PromptData_Results")
PROJECTS = [
    "curl", "gcc", "git", "h2o", "libgit2", 
    "linux", "mysql", "protobuf", "redis", "tmux"
]

def get_project_paths(project_name: str) -> tuple[str, str, str]:
    """
    Get all paths related to the project
    Returns: (project source folder, txt output folder, json output folder)
    """
    source_dir = os.path.join(BASE_RESULTS_DIR, project_name)
    txt_output_dir = os.path.join(PROMTP_RESULTS_DIR, f"{project_name}_prompts")
    json_output_dir = os.path.join(PROMTP_RESULTS_DIR, f"{project_name}_prompts_json")
    return source_dir, txt_output_dir, json_output_dir

def clean_output_folders(project_name: str):
    """Clean output folders for the specified project"""
    _, txt_dir, json_dir = get_project_paths(project_name)
    for folder in [txt_dir, json_dir]:
        if os.path.exists(folder):
            print(f"Cleaning output folder: {folder}")
            shutil.rmtree(folder)
        os.makedirs(folder)
        print(f"Folder recreated: {folder}")

def create_prompt(c_file_path: Path, txt_file_path: Path) -> tuple[str, dict]:
    """
    Create standardized prompt text and JSON format
    Returns: tuple of (txt_content, json_content)
    """
    # Read .c file content
    with open(c_file_path, "r", encoding="utf-8") as f:
        c_content = f.read()
    
    # Read pointer information
    with open(txt_file_path, "r", encoding="utf-8") as f:
        txt_content = f.read()
        
    ptr_info = txt_content.strip().split(",")
    if len(ptr_info) < 7:
        raise ValueError(f"Invalid pointer information format: {txt_file_path}")
        
    ptr1, file1, line1, ptr2, file2, line2, label = ptr_info[:7]
    
    result_map = {
        "1": "MUST",
        "2": "MAY", 
        "0": "MUST-NOT"
    }
    expected_result = result_map.get(label, "Unknown")
    
    # Build question description
    question = (
        f"Below is a function in C. Please analyze the alias relationship between:\n"
        f"1. Pointer '{ptr1}' at line {line1}\n"
        f"2. Pointer '{ptr2}' at line {line2}\n\n"
        f"Determine if these two pointers MUST alias (always point to the same location), "
        f"MAY alias (might point to the same location), or "
        f"MUST-NOT alias (never point to the same location) after the execution of the function.\n\n"
        f"Program to analyze:\n{c_content}"
    )
    
    # Build text format prompt
    txt_prompt = (
        "[ROLE]\n"
        "Pointer analyzer that analyzes whether two pointer in a program alias\n\n"
        "[QUESTION]\n"
        f"{question}\n\n"
        "[ANSWER]\n"
        f"{expected_result}\n"
    )
    
    # Build JSON format prompt
    json_prompt = {
        "ROLE": "Pointer analyzer that analyzes whether two pointer in a program alias",
        "QUESTION": question,
        "ANSWER": expected_result
    }
    
    return txt_prompt, json_prompt

def process_project_files(project_name: str):
    """Process all file pairs in a single project folder"""
    source_dir, txt_output_dir, json_output_dir = get_project_paths(project_name)
    
    if not os.path.exists(source_dir):
        print(f"Error: Project directory does not exist: {source_dir}")
        return
        
    # Clean output directories without printing cleanup info
    clean_output_folders(project_name)
    
    # Get all .c files and pre-filter those without corresponding txt files
    c_files = []
    for c_file in Path(source_dir).glob("*.c"):
        if c_file.with_suffix(".txt").exists():
            c_files.append(c_file)
    
    total_files = len(c_files)
    
    # Use sys.stdout to force flush, ensuring progress bar displays correctly
    import sys
    sys.stdout.flush()
    
    # Configure progress bar, use \r to update on the same line
    with tqdm(total=total_files,
             desc=f"Processing {project_name}",
             bar_format='\r{desc}:|{bar:50}|{percentage:3.0f}%',
             ascii=True,
             ncols=80,
             file=sys.stdout,
             miniters=1) as pbar:
        
        for c_file in c_files:
            try:
                txt_prompt, json_prompt = create_prompt(c_file, c_file.with_suffix(".txt"))
                
                # Save files without printing any information
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
                sys.stdout.flush()  # Force output flush
    
    # Print newline after progress bar ends
    print()

def process_all_projects():
    """Process all project folders"""
    print("Starting batch processing of all project folders")
    
    total_start_time = time.time()
    
    for project in PROJECTS:
        project_start_time = time.time()
        try:
            process_project_files(project)
        except Exception as e:
            print(f"Error processing project {project}: {str(e)}")
        finally:
            project_end_time = time.time()
            print(f"Project {project} processing time: {project_end_time - project_start_time:.2f} seconds")
            
    total_end_time = time.time()
    print(f"\nAll projects completed! Total time: {total_end_time - total_start_time:.2f} seconds")

if __name__ == "__main__":
    process_all_projects()