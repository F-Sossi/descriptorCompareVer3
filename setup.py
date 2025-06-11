import os
import tarfile
import urllib.request
import subprocess  # Import subprocess


def call_resizer():
    """
    Calls the resizer.py script located in the /scripts directory.
    """
    script_path = os.path.join(os.path.dirname(__file__), 'scripts', 'resizer.py')
    try:
        subprocess.run(['python', script_path], check=True)
        print("Resizer script executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Failed to execute resizer script: {e}")

def directory_has_files(dir_path):
    """
    Check if a directory exists and has files other than .gitkeep.
    """
    if not os.path.exists(dir_path):
        return False
    for filename in os.listdir(dir_path):
        if filename != ".gitkeep":
            return True
    return False


def extract_tarfile_skip_top_level(tar_path, extract_to):
    """
    Extract a tar.gz file skipping the top-level directory.
    """
    with tarfile.open(tar_path, "r:gz") as tar:
        members = tar.getmembers()
        # Strip the first directory from the path of all members
        for member in members:
            member.name = os.path.sep.join(member.name.split(os.path.sep)[1:])
            if member.name:  # Skip the top-level directory entry (empty name after stripping)
                tar.extract(member, path=extract_to)


def download_and_extract(url, extract_to='data'):
    """
    Download a .tar.gz file and extract its contents, unless the target directory already contains meaningful data.
    """
    if directory_has_files(extract_to):
        print(f"The '{extract_to}' directory already contains data. Skipping download and extraction.")
        return

    # Make directory if it doesn't exist
    os.makedirs(extract_to, exist_ok=True)

    # Define the name of the tar file
    tar_path = os.path.join(extract_to, url.split('/')[-1])

    # Download the file
    print(f"Downloading {url}...")
    urllib.request.urlretrieve(url, tar_path)
    print("Download complete.")

    # Extract the tar file, skipping the top-level directory
    print(f"Extracting contents to {extract_to}...")
    extract_tarfile_skip_top_level(tar_path, extract_to)
    print("Extraction complete.")

    # Optionally, remove the tar file after extraction
    os.remove(tar_path)
    print("Cleanup complete.")


if __name__ == "__main__":
    url = "http://icvl.ee.ic.ac.uk/vbalnt/hpatches/hpatches-sequences-release.tar.gz"
    download_and_extract(url)
