import os
import subprocess
import shutil

# Define target directory
REPOS_DIR = os.path.join(os.getcwd(), "Repos")

# Define repositories and their versions
REPOSITORIES = {
    "linux": {
        "url": "https://github.com/torvalds/linux.git",
        "version": "v5.3"  # Use correct tag format
    },
    "mysql": {
        "url": "https://github.com/mysql/mysql-server.git",
        "version": "mysql-8.0.25"  # Add mysql- prefix
    },
    "gcc": {
        "url": "https://github.com/gcc-mirror/gcc.git",
        "version": "releases/gcc-10.3.0"
    },
    "git": {
        "url": "https://github.com/git/git.git",
        "version": "f443b2"  # Use complete commit hash
    },
    "tmux": {
        "url": "https://github.com/tmux/tmux.git",
        "version": "5071b82"  # commit hash, no need to modify
    },
    "redis": {
        "url": "https://github.com/redis/redis.git",
        "version": "6.2.6"
    },
    "curl": {
        "url": "https://github.com/curl/curl.git",
        "version": "curl-7_79_0"  # Use correct tag format
    },
    "leveldb": {
        "url": "https://github.com/google/leveldb.git",
        "version": "1.23"  # Remove v prefix, use correct tag format
    },
    "h2o": {
        "url": "https://github.com/h2o/h2o.git",
        "version": "3e4b697"  # commit hash, no need to modify
    },
    "libgit2": {
        "url": "https://github.com/libgit2/libgit2.git",
        "version": "2fc0fcb"  # commit hash, no need to modify
    },
    "the-silver-searcher": {
        "url": "https://github.com/ggreer/the_silver_searcher.git",
        "version": "a61f178"  # commit hash, no need to modify
    },
    "protobuf": {
        "url": "https://github.com/protocolbuffers/protobuf.git",
        "version": "v3.20.0"  # Add v prefix
    },
    "aria2": {
        "url": "https://github.com/aria2/aria2.git",
        "version": "release-1.36.0"  # Use correct tag format
    },
    "fish": {
        "url": "https://github.com/fish-shell/fish-shell.git",
        "version": "3.4.1"
    }
}

def clone_repository(name, info):
    """Clone repository with specific version"""
    url = info["url"]
    version = info["version"]
    temp_dir = f"temp_{name}"
    
    # Define repositories using commit hash
    commit_hash_repos = {
        "git": "f443b2",
        "tmux": "5071b82",
        "h2o": "3e4b697",
        "libgit2": "2fc0fcb",
        "the-silver-searcher": "a61f178"
    }
    
    print(f"\nProcessing {name}...")
    
    try:
        # Ensure temporary directory doesn't exist
        if os.path.exists(temp_dir):
            print(f"Removing existing {temp_dir}...")
            shutil.rmtree(temp_dir)
        
        if name == "linux":
            print("Using special clone method for Linux...")
            # Use -c core.ignorecase=true to avoid case conflicts
            subprocess.run(["git", "-c", "core.ignorecase=true", 
                          "clone", "--depth", "1", 
                          "--branch", version, url, temp_dir], 
                         check=True)
        elif name in commit_hash_repos:
            print(f"Using commit hash clone method for {name}...")
            # Full repository clone
            subprocess.run(["git", "clone", url, temp_dir], 
                         check=True)
            
            # Switch to specified commit
            subprocess.run(["git", "checkout", version],
                         cwd=temp_dir, check=True)
            
        else:
            print(f"Trying to clone {name} with version {version}...")
            try:
                # First try using version number directly
                subprocess.run(["git", "clone", "--branch", version, 
                              "--depth", "1", url, temp_dir], 
                             check=True)
            except subprocess.CalledProcessError:
                print(f"Failed with version {version}, trying with alternative formats...")
                # Try different version number formats
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
        
        # Move to final location
        final_path = os.path.join(REPOS_DIR, name)
        if os.path.exists(final_path):
            print(f"Removing existing {final_path}...")
            shutil.rmtree(final_path)
        shutil.move(temp_dir, final_path)
        
        print(f"Successfully downloaded {name}")
        
    except Exception as e:
        print(f"Error processing {name}: {str(e)}")
        # Clean up failed temporary directory
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)

def main():
    # Ensure Repos directory exists
    os.makedirs(REPOS_DIR, exist_ok=True)
    
    # Get list of existing repositories
    existing_repos = set(os.listdir(REPOS_DIR))
    
    # Download missing repositories
    for name, info in REPOSITORIES.items():
        if name not in existing_repos:
            print(f"\nRepository {name} not found, downloading...")
            clone_repository(name, info)
        else:
            print(f"\nSkipping {name} - already exists")
    
    print("\nDownload completed!")

if __name__ == "__main__":
    main()