from openai import OpenAI
import logging
import re
import os
from pathlib import Path

global_model = "openai/gpt-4o"  
secrete = ""
PROMPT_FOLDER = "/mnt/d/CodeSemResults/Results/curl_prompts"
RESULT_FOLDER = "/mnt/d/CodeSemResults/Results/curl_analysis"

class PointerAnalysisLLM:
    def __init__(self, api_key: str = secrete):
        self.set_up_openrouter_configs()
        self.system_prompt = self._get_system_prompt()
        
    def set_up_openrouter_configs(self):
        try:
            self.client = OpenAI(
                api_key=secrete,
                base_url="https://openrouter.ai/api/v1",
                timeout=30
            )
        except Exception as e:
            logging.error(f"OpenRouter configuration failed: {e}")
            raise

    def _get_system_prompt(self) -> str:
        return """You are a pointer analysis expert focusing on C program alias analysis.

Core Analysis Principles:
1. Memory Location Focus
   - Track where each pointer points to
   - Consider all possible runtime values
   - Analyze memory access patterns

2. Conservative Analysis
   - Array indices are unknown at runtime
   - Uninitialized pointers are undefined
   - Multiple execution paths need consideration

3. Struct Field Rules
   - Different fields never alias
   - Same field may alias across instances
   - Field access patterns matter

4. Array Handling
   - Array elements may alias
   - Array indices are dynamic
   - Conservative array analysis required

5. Assignment Rules
   - Direct assignments create must-alias
   - Indirect assignments may create aliases
   - Consider all assignment paths

Your task is to determine if two pointers:
- MUST alias (always point to same location)
- MAY alias (might point to same location)
- NO alias (never point to same location)"""

    def preprocess_program(self, program: str) -> str:
        # Remove C-style comments
        program = re.sub(r'/\*.*?\*/', '', program, flags=re.DOTALL)
        program = re.sub(r'//.*$', '', program, flags=re.MULTILINE)
        
        # Mark pointer operations and address-of operations
        program = re.sub(r'([a-zA-Z_][a-zA-Z0-9_]*\s*(?:\[.*?\])?\s*(?:->|\.)\s*[a-zA-Z_][a-zA-Z0-9_]*)',
                        r'POINTER_OP[\1]', program)
        program = re.sub(r'(&[a-zA-Z_][a-zA-Z0-9_]*)', r'ADDRESS_OF[\1]', program)
        
        return program

    def analyze_pointers(self, program: str, ptr1: str, ptr2: str) -> str:
        try:
            processed_program = self.preprocess_program(program)
            content = f"""Given a C program with pointer analysis task:

Program:
{processed_program}

Task:
Analyze the aliasing relationship between pointers {ptr1} and {ptr2}.

Key Rules to Consider:
1. Array elements with different indices MAY alias (array indices are uncertain at runtime)
2. Same fields in different struct array elements MAY alias (conservative array handling)
3. Different struct fields NEVER alias (field sensitivity)
4. Uninitialized pointers are undefined
5. After direct assignment p = q, p and q MUST alias

You must output in exactly this format:
1. POINTERS: {ptr1},{ptr2}
2. RESULT: [MAY|NO|MUST]
3. REASON: [Single clear sentence explaining the result]

Guidelines for determining RESULT:
- MUST = Pointers are guaranteed to point to same address (100% certain)
- MAY = Pointers could potentially point to same address
- NO = Pointers definitely never point to same address

Analyze step by step:
1. Track all assignments to both pointers
2. Consider all possible execution paths
3. Apply pointer analysis rules
4. Make conservative assumptions for uncertainty
"""
            response = self.client.chat.completions.create(
                model=global_model,
                messages=[
                    {"role": "system", "content": self.system_prompt},
                    {"role": "user", "content": content}
                ],
                max_tokens=1000,
                temperature=0.1
            )
            result = response.choices[0].message.content
            print(f"LLM Analysis for {ptr1} and {ptr2}:\n{result}")
            return result
        except Exception as e:
            logging.error(f"LLM analysis failed: {str(e)}")
            return None

    def analyze_pointer_alias_from_prompt(self, prompt_file: Path) -> dict:
        """
        Analyze pointer aliasing from a prompt file
        Returns analysis result as a dictionary
        """
        try:
            # Read prompt file content
            with open(prompt_file, "r", encoding="utf-8") as f:
                content = f.read()

            # Parse the prompt sections
            sections = content.split("[")
            program_section = ""
            answer_section = ""
            
            for section in sections:
                if section.startswith("QUESTION]"):
                    program_section = section.replace("QUESTION]", "").strip()
                elif section.startswith("ANSWER]"):
                    answer_section = section.replace("ANSWER]", "").strip()

            # Parse the answer section to get pointers
            ptr_info = answer_section.strip().split(",")
            if len(ptr_info) < 4:
                raise ValueError(f"Invalid answer format in {prompt_file}")

            ptr1, file1, line1, ptr2, file2, line2, expected_result = ptr_info[:7]

            # Perform analysis
            analysis_result = self.analyze_pointers(program_section, ptr1, ptr2)
            
            return {
                "file": str(prompt_file),
                "ptr1": ptr1,
                "ptr2": ptr2,
                "expected": expected_result,
                "analysis": parse_llm_response(analysis_result)
            }

        except Exception as e:
            logging.error(f"Analysis failed for {prompt_file}: {str(e)}")
            return None

def parse_llm_response(response: str) -> dict:
    if not response:
        return {"result": "", "reason": "No analysis available"}
    
    lines = response.strip().split('\n')
    result = {"result": "", "reason": "No analysis available"}
    
    for line in lines:
        line = line.strip()
        if line.startswith('2. RESULT:'):
            result_text = line.split(':')[1].strip()
            if result_text in ["MAY", "NO", "MUST"]:
                result['result'] = result_text
        elif line.startswith('3. REASON:'):
            result['reason'] = line.split(':')[1].strip()
    
    return result

def process_all_prompts():
    """
    Process all prompt files in the curl_prompts directory
    """
    os.makedirs(RESULT_FOLDER, exist_ok=True)
    llm = PointerAnalysisLLM()
    
    # Get all prompt files
    prompt_files = list(Path(PROMPT_FOLDER).glob("*_prompt.txt"))
    total_files = len(prompt_files)
    
    print(f"Found {total_files} prompt files to process")
    
    summary_file = Path(RESULT_FOLDER) / "analysis_summary.txt"
    with open(summary_file, "w", encoding="utf-8") as summary:
        summary.write("Pointer Analysis Results Summary\n")
        summary.write("=" * 50 + "\n\n")
        
        for i, prompt_file in enumerate(prompt_files, 1):
            print(f"\nProcessing file {i}/{total_files}: {prompt_file.name}")
            
            try:
                # Analyze the prompt
                result = llm.analyze_pointer_alias_from_prompt(prompt_file)
                
                if result:
                    # Write individual result file
                    result_file = Path(RESULT_FOLDER) / f"{prompt_file.stem}_result.txt"
                    with open(result_file, "w", encoding="utf-8") as rf:
                        rf.write(f"Analysis Result for {prompt_file.name}\n")
                        rf.write("=" * 50 + "\n\n")
                        rf.write(f"Pointer 1: {result['ptr1']}\n")
                        rf.write(f"Pointer 2: {result['ptr2']}\n")
                        rf.write(f"Expected Result: {result['expected']}\n")
                        rf.write(f"Analysis Result: {result['analysis']['result']}\n")
                        rf.write(f"Analysis Reason: {result['analysis']['reason']}\n")
                    
                    # Add to summary
                    summary.write(f"File: {prompt_file.name}\n")
                    summary.write(f"Result: {result['analysis']['result']} (Expected: {result['expected']})\n")
                    summary.write("-" * 50 + "\n\n")
                    
                    print(f"Analysis completed for {prompt_file.name}")
                    
            except Exception as e:
                print(f"Error processing {prompt_file.name}: {str(e)}")
                continue
    
    print(f"\nAnalysis complete. Results saved to {RESULT_FOLDER}")

if __name__ == "__main__":
    # Set up logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s'
    )
    
    # Process all prompts
    process_all_prompts()