import struct

MKDIR_OPCODE = 0x09  # Define the MKDIR_OPCODE


def recv_all(sock, length):
    data = b''
    while len(data) < length:
        packet = sock.recv(length - len(data))
        if not packet:
            return None  # Connection closed
        data += packet
    return data


def send_mkdir_request(sock, session_id, directory_name):
    # specified format.
    opcode = MKDIR_OPCODE
    reserved = 0x00
    directory_name_bytes = directory_name.encode('utf-8')
    dir_name_length = len(directory_name_bytes)

    # Create the request packet
    request = struct.pack(f"!BBH4s4x{dir_name_length}s", opcode, reserved, dir_name_length,
                          session_id.to_bytes(4, 'big'), directory_name_bytes)

    # Send the request
    print(f"Sending request to create directory '{directory_name}'")
    sock.sendall(request)

    # Receive the response
    response = recv_all(sock, 8)

    # Unpack the response (first 6 bytes contain the meaningful data)
    if response and len(response) >= 6:
        # Unpack the return_code from the first byte
        return_code, = struct.unpack("!B", response[:1])

        if return_code == 0x01:
            print(f"Directory '{directory_name}' created successfully.")
        elif return_code == 0xFF:
            print(f"Failed to create directory '{directory_name}'.")
        elif return_code == 0x02:
            print("Provided Session ID was invalid or expired")
        elif return_code == 0x03:
            print("User associated with provided Session ID had insufficient permissions to make a new directory")
        else:
            print("Unknown return code from server.")
    else:
        print("Received data is too short or incorrectly formatted.")
