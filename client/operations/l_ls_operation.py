import os


# This was pretty easy.
def list_local_directory(path=None):
    if path is None:
        path = os.getcwd()

    try:
        with os.scandir(path) as entries:
            for entry in entries:
                if entry.is_dir():
                    print(f"[DIR]  {entry.name}")
                else:
                    print(f"[FILE] {entry.name}")
    except FileNotFoundError:
        print(f"Error: The directory '{path}' was not found.")
    except PermissionError:
        print(f"Error: Permission denied to access '{path}'.")
    except Exception as e:
        print(f"An error occurred: {e}")
