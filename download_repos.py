import os
import subprocess
import shutil

# 定义目标目录
REPOS_DIR = os.path.join(os.getcwd(), "Repos")

# 定义要下载的仓库及其版本
REPOSITORIES = {
    "linux": {
        "url": "https://github.com/torvalds/linux.git",
        "version": "v5.3"  # 修改为正确的标签格式
    },
    "mysql": {
        "url": "https://github.com/mysql/mysql-server.git",
        "version": "mysql-8.0.25"  # 添加 mysql- 前缀
    },
    "gcc": {
        "url": "https://github.com/gcc-mirror/gcc.git",
        "version": "releases/gcc-10.3.0"
    },
    "git": {
        "url": "https://github.com/git/git.git",
        "version": "f443b2"  # 使用完整的commit hash
    },
    "tmux": {
        "url": "https://github.com/tmux/tmux.git",
        "version": "5071b82"  # commit hash，无需修改
    },
    "redis": {
        "url": "https://github.com/redis/redis.git",
        "version": "6.2.6"
    },
    "curl": {
        "url": "https://github.com/curl/curl.git",
        "version": "curl-7_79_0"  # 修改为正确的标签格式
    },
    "leveldb": {
        "url": "https://github.com/google/leveldb.git",
        "version": "1.23"  # 移除 v 前缀，使用正确的标签格式
    },
    "h2o": {
        "url": "https://github.com/h2o/h2o.git",
        "version": "3e4b697"  # commit hash，无需修改
    },
    "libgit2": {
        "url": "https://github.com/libgit2/libgit2.git",
        "version": "2fc0fcb"  # commit hash，无需修改
    },
    "the-silver-searcher": {
        "url": "https://github.com/ggreer/the_silver_searcher.git",
        "version": "a61f178"  # commit hash，无需修改
    },
    "protobuf": {
        "url": "https://github.com/protocolbuffers/protobuf.git",
        "version": "v3.20.0"  # 添加 v 前缀
    },
    "aria2": {
        "url": "https://github.com/aria2/aria2.git",
        "version": "release-1.36.0"  # 修改为正确的标签格式
    },
    "fish": {
        "url": "https://github.com/fish-shell/fish-shell.git",
        "version": "3.4.1"
    }
}

def clone_repository(name, info):
    """克隆特定版本的代码库"""
    url = info["url"]
    version = info["version"]
    temp_dir = f"temp_{name}"
    
    # 定义使用 commit hash 的仓库列表
    commit_hash_repos = {
        "git": "f443b2",
        "tmux": "5071b82",
        "h2o": "3e4b697",
        "libgit2": "2fc0fcb",
        "the-silver-searcher": "a61f178"
    }
    
    print(f"\nProcessing {name}...")
    
    try:
        # 确保临时目录不存在
        if os.path.exists(temp_dir):
            print(f"Removing existing {temp_dir}...")
            shutil.rmtree(temp_dir)
        
        if name == "linux":
            print("Using special clone method for Linux...")
            # 使用 -c core.ignorecase=true 避免大小写冲突
            subprocess.run(["git", "-c", "core.ignorecase=true", 
                          "clone", "--depth", "1", 
                          "--branch", version, url, temp_dir], 
                         check=True)
        elif name in commit_hash_repos:
            print(f"Using commit hash clone method for {name}...")
            # 完整克隆仓库
            subprocess.run(["git", "clone", url, temp_dir], 
                         check=True)
            
            # 切换到指定的 commit
            subprocess.run(["git", "checkout", version],
                         cwd=temp_dir, check=True)
            
        else:
            print(f"Trying to clone {name} with version {version}...")
            try:
                # 首先尝试直接使用版本号
                subprocess.run(["git", "clone", "--branch", version, 
                              "--depth", "1", url, temp_dir], 
                             check=True)
            except subprocess.CalledProcessError:
                print(f"Failed with version {version}, trying with alternative formats...")
                # 尝试不同的版本号格式
                alternate_versions = [
                    version,
                    f"v{version}",
                    version.replace("v", "")
                ]
                cloned = False
                for alt_version in alternate_versions:
                    try:
                        if os.path.exists(temp_dir):
                            shutil.rmtree(temp_dir)
                        subprocess.run(["git", "clone", "--branch", alt_version, 
                                      "--depth", "1", url, temp_dir], 
                                     check=True)
                        cloned = True
                        break
                    except subprocess.CalledProcessError:
                        continue
                
                if not cloned:
                    raise Exception(f"Failed to clone {name} with any version format")
        
        # 移动到最终位置
        final_path = os.path.join(REPOS_DIR, name)
        if os.path.exists(final_path):
            print(f"Removing existing {final_path}...")
            shutil.rmtree(final_path)
        shutil.move(temp_dir, final_path)
        
        print(f"Successfully downloaded {name}")
        
    except Exception as e:
        print(f"Error processing {name}: {str(e)}")
        # 清理失败的临时目录
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)

def main():
    # 确保 Repos 目录存在
    os.makedirs(REPOS_DIR, exist_ok=True)
    
    # 获取已存在的仓库列表
    existing_repos = set(os.listdir(REPOS_DIR))
    
    # 下载缺失的仓库
    for name, info in REPOSITORIES.items():
        if name not in existing_repos:
            print(f"\nRepository {name} not found, downloading...")
            clone_repository(name, info)
        else:
            print(f"\nSkipping {name} - already exists")
    
    print("\nDownload completed!")

if __name__ == "__main__":
    main()