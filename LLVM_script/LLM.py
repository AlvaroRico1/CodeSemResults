from openai import OpenAI
import logging
import re

global_model = "openai/gpt-4o"  
secrete = ""

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