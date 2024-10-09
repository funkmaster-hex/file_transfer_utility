import os


# This was pretty easy. 
def make_local_directory(path=None):
    if path is None:
        path = os.getcwd()
    try:
        os.makedirs(path, exist_ok=True)
        print(f"Directory '{path}' created successfully.")
    except PermissionError:
        print(f"Error: Permission denied to create directory '{path}'.")
    except Exception as e:
        print(f"An error occurred: {e}")
