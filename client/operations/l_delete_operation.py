import os
import shutil


# This works far too well. No warnings and it's super easy to make a mistake.
# I lost an entire day's work after I deleted an entire client.

def delete_local_path(path=None):
    if path is None:
        path = os.getcwd()

    try:
        if os.path.isdir(path):
            shutil.rmtree(path)
            print(f"Directory '{path}' deleted successfully.")
        elif os.path.isfile(path):
            os.remove(path)
            print(f"File '{path}' deleted successfully.")
        else:
            print(f"Error: '{path}' does not exist.")
    except FileNotFoundError:
        print(f"Error: The path '{path}' was not found.")
    except PermissionError:
        print(f"Error: Permission denied to delete '{path}'.")
    except Exception as e:
        print(f"An error occurred: {e}")
