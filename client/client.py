import socket

from operations.user_operation import handle_user_operation
from operations.util.session import get_session_id, set_session_id
from operations.ls_operation import send_ls_request
from operations.get_operation import send_get_request
from operations.put_operation import send_put_request
from operations.l_ls_operation import list_local_directory
from operations.l_delete_operation import delete_local_path
from operations.l_mkdir_operation import make_local_directory
from operations.delete_operation import send_delete_request
from operations.mkdir_operation import send_mkdir_request


commands = {
    'get': 'Gets a file from server [src] path and copies it into the client [dst] path',
    'put': 'Sends a file from client [src] path to be placed in the server [dst] path',
    'delete': 'Deletes file at server [path]',
    'l_delete': 'Deletes file at local [path]',
    'ls': 'Lists remote directory contents',
    'l_ls': 'Lists local directory contents',
    'mkdir': 'Makes directory at server [path]',
    'l_mkdir': 'Makes directory at client [path]',
    'help': 'Print available commands'
}
SERVER_PORT = 0


def main():
    global SERVER_PORT
    print("Welcome... let's get started")
    set_session_id(0)
    while True:
        SERVER_IP = input("Enter server IP: ")

        try:
            socket.inet_aton(SERVER_IP)
        except socket.error:
            print("Invalid IP address format. Please enter in the format '127.0.0.1'.")
            continue

        SERVER_PORT = input("Enter server port: ")

        # Check if port is a digit and within the valid range (1-65535)
        if not SERVER_PORT.isdigit() or not (1 <= int(SERVER_PORT) <= 65535):
            print("Server port should be a number between 1 and 65535.")
            continue

        SERVER_PORT = int(SERVER_PORT)  # Convert to an integer after validation
        break

    print(f"Using server IP: {SERVER_IP} and port: {SERVER_PORT}")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((SERVER_IP, SERVER_PORT))

        session_id = get_session_id()
        if session_id == 0:
            handle_user_operation(sock, session_id)
            session_id = get_session_id()
            if session_id == 0:
                print("Session ID not set. Exiting...")
                return

        print(f"Connected to server with session ID: {session_id}")

        while True:
            print("\nChoose an operation:")
            print("1: User operation")
            print("2: Delete remote file")
            print("3: List remote directory")
            print("4: Get remote file")
            print("5: Make remote directory")
            print("6: Put remote file")
            print("7: l_ls")
            print("8: l_delete")
            print("9: l_mkdir")
            print("10: Help")
            print("0: Exit")

            operation = input("Enter operation number: ")
            if operation.isdigit():
                operation = int(operation)
            else:
                print("Invalid input, please enter a number.")
                continue

            if operation == 0:
                print("Exiting...")
                break
            if operation == 1:  # User operations
                handle_user_operation(sock, session_id)
            elif operation == 2:  # handle remote delete
                remote_file = input("Enter remote file path to delete: ")
                send_delete_request(sock, session_id, remote_file)
            elif operation == 3:  # handle remote ls
                directory_name = input("Enter directory name (or leave blank for root): ")
                send_ls_request(sock, session_id, directory_name)
            elif operation == 4:  # handle get request
                remote_file = input("Enter remote file path: ")
                local_file = input("Enter local file path to save: ")
                send_get_request(sock, session_id, remote_file, local_file)
            elif operation == 5:  # Handle Make remote directory
                directory_name = input("Enter directory name to create: ")
                send_mkdir_request(sock, session_id, directory_name)
            elif operation == 6:  # handle put request
                local_file = input("Enter local file path to upload: ")
                remote_file = input("Enter remote file path to save: ")
                overwrite_flag = input("Overwrite existing file? (yes/no): ").strip().lower() == 'yes'
                send_put_request(sock, session_id, local_file, remote_file, overwrite_flag)
            elif operation == 7:  # local ls
                local_path = input("Enter path for local ls (or leave blank for current directory): ")
                list_local_directory(local_path if local_path else None)
            elif operation == 8:  # local delete
                local_path = input("Enter path for local delete (or leave blank for current directory): ")
                delete_local_path(local_path if local_path else None)
            elif operation == 9:  # local mkdir
                local_path = input("Enter path for local mkdir (or leave blank for current directory): ")
                make_local_directory(local_path if local_path else None)
            elif operation == 10:  # Display help menu
                for command, description in commands.items():
                    print(f"{command}: {description}\n")
            else:
                print("Operation not implemented yet.")


if __name__ == "__main__":
    main()
