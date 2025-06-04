import os
import math
import shutil

PROMPT_FOLDER = os.path.join(os.getcwd(), "PromptData_Results")
TRAINING_SET_FOLDER = os.path.join(os.getcwd(), "TrainingSet")
VALIDATE_SET_FOLDER = os.path.join(os.getcwd(), "ValidatingSet")

if __name__ == "__main__":
    # 确保目标文件夹存在
    os.makedirs(TRAINING_SET_FOLDER, exist_ok=True)
    os.makedirs(VALIDATE_SET_FOLDER, exist_ok=True)

    for item_folder in os.listdir(PROMPT_FOLDER):
        if "_json" not in item_folder:
            continue

        folder_path = os.path.join(PROMPT_FOLDER, item_folder)
        # 只列出文件，不包含子目录
        all_names = os.listdir(folder_path)
        filenames = [
            name for name in sorted(all_names)
            if os.path.isfile(os.path.join(folder_path, name))
        ]

        file_num = len(filenames)
        if file_num == 0:
            continue

        split_index = file_num * 7 // 10  # 等同于 math.floor(file_num * 0.7)

        for idx, filename in enumerate(filenames):
            src_path = os.path.join(folder_path, filename)
            if idx < split_index:
                dst_path = os.path.join(TRAINING_SET_FOLDER, filename)
            else:
                dst_path = os.path.join(VALIDATE_SET_FOLDER, filename)

            # 直接用 Python API 拷贝，避免 shell 解析问题
            try:
                shutil.copy2(src_path, dst_path)
            except FileNotFoundError:
                print(f"Warning: 源文件不存在，跳过：{src_path}")
            except Exception as e:
                print(f"Error copying {src_path} → {dst_path}: {e}")

        print(f"Finished splitting '{item_folder}': total={file_num}, "
              f"train={split_index}, validate={file_num - split_index}")