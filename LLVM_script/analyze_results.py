import os
from pathlib import Path

import sys
import re
from pathlib import Path
from openai import OpenAI
import logging

print("here")
PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
STATIC_SOURCE_FOLDER = os.path.join(PROJECT_DIR, "static_file_data/src")
TARGET_LABELLED_SRC = os.path.join(PROJECT_DIR, "files_with_label")
TRAINING_DATA_SRC_FOLDER = os.path.join(PROJECT_DIR, "training_data/src")
TRAINING_DATA_PROMPT_FOLDER = os.path.join(PROJECT_DIR, "training_data/prompt")
RESULT_FOLDER = os.path.join(PROJECT_DIR, "result")


os.makedirs(RESULT_FOLDER, exist_ok=True)
def analyze_results():
    """Analyze result files and calculate accuracy"""
    result_dir = Path("result")
    total_pairs = 0
    correct_pairs = 0
    
    # Statistics for each combination
    combination_stats = {}
    
    print("Starting analysis...")
    print("-" * 50)
    
    # Check if directory exists
    if not result_dir.exists():
        print(f"Error: Directory '{result_dir}' not found")
        return
    
    # Get all txt files
    result_files = list(result_dir.glob("*_analysis.txt"))
    if not result_files:
        print("Error: No analysis files found")
        return
        
    for file_path in result_files:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()
            
            # Find Summary section
            if "Summary (Ground Truth,LLM Result):" in content:
                summary_section = content.split("Summary (Ground Truth,LLM Result):")[1].strip()
                pairs = summary_section.strip().split("\n")
                
                file_correct = 0
                file_total = len(pairs)
                
                print(f"\nAnalyzing file: {file_path.name}")
                print(f"Found {file_total} comparison results")
                
                for pair in pairs:
                    if pair.strip():  # Skip empty lines
                        ground_truth, llm_result = pair.strip().split(",")
                        
                        # Count combination occurrences
                        combo = f"{ground_truth}-{llm_result}"
                        combination_stats[combo] = combination_stats.get(combo, 0) + 1
                        
                        # Calculate correct predictions
                        if ground_truth == llm_result:
                            file_correct += 1
                            correct_pairs += 1
                        
                        total_pairs += 1
                
                print(f"File accuracy: {file_correct}/{file_total} = {(file_correct/file_total)*100:.2f}%")
    
    print("\n" + "="*50)
    print("Overall statistics:")
    print(f"Total samples: {total_pairs}")
    print(f"Correct predictions: {correct_pairs}")
    print(f"Overall accuracy: {(correct_pairs/total_pairs)*100:.2f}%")
    
    print("\nPrediction result distribution:")
    for combo, count in sorted(combination_stats.items()):
        ground_truth, llm_result = combo.split("-")
        print(f"Ground Truth: {ground_truth:4}, LLM Result: {llm_result:4} - Occurrences: {count:3}")

if __name__ == "__main__":
    analyze_results()