import os
import csv
import re
import subprocess

def generate_tags(curl_path):
    """Generate function location information using ctags"""
    # Set the full path of ctags executable
    ctags_exe = "/usr/bin/ctags"  # ctags path in WSL
    
    # Create .ctags configuration file
    with open('.ctags', 'w') as f:
        f.write('--languages=C\n')
        f.write('--kinds-C=f\n')

    # Run ctags command
    try:
        subprocess.run([ctags_exe, '-R', '--fields=+n', '-f', '.tags', curl_path], 
                      check=True, 
                      capture_output=True)
    except subprocess.CalledProcessError as e:
        print(f"Ctags execution error: {e.stderr.decode()}")
        raise
    except FileNotFoundError:
        print(f"Cannot find Ctags executable: {ctags_exe}")
        print("Please make sure Universal Ctags is installed")
        raise
    
    # Read and parse tags file
    function_ranges = {}
    current_file = None
    current_start = None
    
    with open('.tags', 'r', encoding='utf-8') as f:
        for line in f:
            if line.startswith('!'):
                continue
            parts = line.strip().split('\t')
            if len(parts) >= 4:
                func_name = parts[0]
                file_path = parts[1]
                # Extract line number
                line_match = re.search(r'line:(\d+)', line)
                if line_match:
                    line_num = int(line_match.group(1))
                    if file_path not in function_ranges:
                        function_ranges[file_path] = []
                    function_ranges[file_path].append((line_num, func_name))

    return function_ranges

def is_same_function(file_path, line1, line2, function_ranges):
    """Check if two lines are in the same function"""
    if file_path not in function_ranges:
        return False
        
    ranges = function_ranges[file_path]
    # Sort by line number
    ranges.sort(key=lambda x: x[0])
    
    # Find the function containing these lines
    func_line = None
    func_name = None
    
    for start_line, name in ranges:
        if start_line <= min(line1, line2):
            func_line = start_line
            func_name = name
        elif start_line > max(line1, line2):
            break
            
    return func_line is not None

def extract_function_content(file_path, start_line, end_line, function_ranges):
    """Extract function content from specified line range in file"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        if not is_same_function(file_path, start_line, end_line, function_ranges):
            return None
            
        # Find function start line
        for start, _ in function_ranges[file_path]:
            if start <= start_line:
                function_start = start - 1
                
        # Search downward for function end
        current_line = end_line - 1
        brace_count = 1
        while current_line < len(lines):
            line = lines[current_line]
            brace_count += line.count('{') - line.count('}')
            if brace_count == 0:
                function_end = current_line + 1
                break
            current_line += 1
            
        if current_line >= len(lines):
            return None
            
        return ''.join(lines[function_start:function_end])
    except Exception as e:
        print(f"Error processing {file_path}: {str(e)}")
        return None

def main():
    # Create results directory
    os.makedirs("Results", exist_ok=True)
    
    # Read CSV file - using Linux path format
    csv_path = "/mnt/d/CodeSem/CodeSem/datasets/alias_prediction/fine-tune/curl.csv"
    curl_base_path = "/mnt/d/CodeSem/CodeSem/curl-curl-7_79_0"
    
    # Generate function location information
    function_ranges = generate_tags(curl_base_path)
    
    # Dictionary to track appearance count of each source file
    file_counters = {}
    
    with open(csv_path, 'r', encoding='utf-8') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # Skip header row
        
        for i, row in enumerate(reader, 1):
            file1, line1 = row[1], int(row[2])
            file2, line2 = row[4], int(row[5])
            
            # Ensure line numbers are in order
            start_line = min(line1, line2)
            end_line = max(line1, line2)
            
            # Construct full file path
            relative_path = file1.replace('curl/', '').replace('/', os.sep)
            file_path = os.path.join(curl_base_path, relative_path)
            
            # Extract function content
            content = extract_function_content(file_path, start_line, end_line, function_ranges)
            if content:
                # Get function name
                func_name = None
                for start, name in function_ranges[file_path]:
                    if start <= start_line:
                        func_name = name
                    elif start > end_line:
                        break
                
                if func_name:
                    # Get base file name
                    base_name = os.path.splitext(os.path.basename(file1))[0]
                    
                    # Update file counter
                    if base_name not in file_counters:
                        file_counters[base_name] = 1
                    
                    # Generate base filename without extension
                    base_filename = f"{i}_{base_name}_{func_name}_{file_counters[base_name]}"
                    
                    # Save .c file
                    c_filename = os.path.join("Results", f"{base_filename}.c")
                    with open(c_filename, 'w', encoding='utf-8') as f:
                        f.write(f"// Source: {file1}\n")
                        f.write(f"// Lines {start_line}-{end_line}\n")
                        f.write(content)
                    
                    # Save .txt file with same base name
                    txt_filename = os.path.join("Results", f"{base_filename}.txt")
                    with open(txt_filename, 'w', encoding='utf-8') as f:
                        f.write(','.join(row))
                    
                    # Increment counter after both files are created
                    file_counters[base_name] += 1

if __name__ == "__main__":
    main()
    print("Complete! Please check the Results folder.")