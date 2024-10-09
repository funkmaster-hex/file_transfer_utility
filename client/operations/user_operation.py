import struct
from operations.util.session import set_session_id


def handle_user_operation(sock, session_id):
    print("User Operations:")
    print("1: Login")
    print("2: Create User")
    print("3: Delete User")
    user_choice = input("Enter operation number: ")

    if user_choice == "1":
        username = input("Enter username: ")
        password = input("Enter password: ")
        session_id = send_login_request(sock, username, password)
        if session_id is None:
            print("Exiting due to login failure.")
            return
        set_session_id(session_id)

    elif user_choice == "2":
        print("Creating user operations:")
        print("Select level to create user")
        print("1: Create the given user with Read permissions")
        print("2: Create the given user with Read/Write permissions")
        print("3: Create the given user with Admin permissions")
        choice = input("Enter level of operation: ")
        if choice in ["1", "2", "3"]:
            permissions = {
                "1": 0x01,  # Read
                "2": 0x03,  # Read/Write
                "3": 0x04  # Admin
            }
            username = input("Enter username: ")
            password = input("Enter password: ")
            send_create_user_request(sock, session_id, username, password, permissions[choice])

    elif user_choice == "3":
        username = input("Enter username: ")
        send_delete_user_request(sock, session_id, username)
    else:
        print("Invalid operation.")


def print_buffer(buffer):
    print("Buffer to be sent:")
    print(" ".join(f"{b:02x}" for b in buffer))


def send_login_request(sock, username, password):
    username_bytes = username.encode('utf-8')
    password_bytes = password.encode('utf-8')

    session_id_bytes = b'\x00\x00\x00\x00'

    # Pack the request
    request_format = f'>B B 2x H H 4s {len(username_bytes)}s {len(password_bytes)}s'
    request = struct.pack(request_format, 0x01, 0x00, len(username_bytes), len(password_bytes), session_id_bytes,
                          username_bytes, password_bytes)

    # Print the buffer before sending
    print_buffer(request)

    # Send request
    sock.sendall(request)

    # Handle response from server
    response = sock.recv(8)
    return_code, reserved, session_id = struct.unpack('>B B 4s 2x', response)
    session_id = int.from_bytes(session_id, byteorder='big')

    if return_code == 0x01:
        print("Login successful.")
        set_session_id(session_id)
        return session_id
    else:
        print("Login failed.")
        return None


def send_create_user_request(sock, session_id, username, password, flag):
    print(flag)
    username_bytes = username.encode('utf-8')
    password_bytes = password.encode('utf-8')

    request_format = f'>B B 2x H H 4s {len(username_bytes)}s {len(password_bytes)}s'
    request = struct.pack(request_format, 0x01, flag, len(username_bytes), len(password_bytes)
                          , session_id.to_bytes(4, 'big'), username_bytes, password_bytes)

    # Print the buffer before sending
    print_buffer(request)

    # Send request
    sock.sendall(request)

    # Handle response from server
    response = sock.recv(1024)
    if response[0] == 0x01:
        print("User created successfully.")
    else:
        print("User creation failed.")


def send_delete_user_request(sock, session_id, username):
    username_bytes = username.encode('utf-8')
    password_bytes = b''  # Empty password for deletion
    password_len = 0

    request_format = f'>B B 2x H H 4s {len(username_bytes)}s {len(password_bytes)}s'
    # Create the request buffer
    request = struct.pack(request_format, 0x01, 0xff, len(username_bytes), len(password_bytes)
                          , session_id.to_bytes(4, 'big'), username_bytes, password_bytes)

    # Print the buffer before sending
    print_buffer(request)

    # Send request
    sock.sendall(request)

    # Handle response from server
    response = sock.recv(1024)
    if response[0] == 0x00:
        print("User deleted successfully.")
    else:
        print("User deletion failed.")
