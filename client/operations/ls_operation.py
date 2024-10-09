import struct
import socket


def recv_all(sock, length, chunk_size=1024):
    data = b''
    while len(data) < length:
        to_receive = min(length - len(data), chunk_size)
        packet = sock.recv(to_receive)
        if not packet:
            print("Connection closed before all data was received.")
            return None
        data += packet
        print(f"Received chunk of size: {len(packet)} bytes")
    return data


def receive_response(sock):
    # Read the header first
    header_size = 8  # Correct header size based on your description
    header = recv_all(sock, header_size)

    if not header:
        print("Failed to receive header.")
        return

    print(f"Received header: {header.hex()}")

    try:
        # Correctly unpack the 8-byte header
        return_code, reserved, total_length = struct.unpack('>B B I', header[:6])
    except struct.error as e:
        print(f"Error unpacking header: {e}")
        return

    print(f"Return code: {return_code}, Total length: {total_length}")

    if return_code == 0x01:
        if total_length > 0:
            # Read the data
            print(f"Expecting to receive {total_length} bytes of data.")
            data = recv_all(sock, total_length)
            if data is None:
                print("Failed to receive data.")
                return

            if len(data) < total_length:
                print(f"Received data is shorter than expected. Expected: {total_length}, Got: {len(data)}")
                return

            print(f"Received data: {data.hex()}")
            process_data(data)
        else:
            print("No data received.")
    elif return_code == 0xFF:
        print(f"Failed to read and list directory.")
    elif return_code == 0x02:
        print("Provided Session ID was invalid or expired.")
    elif return_code == 0x03:
        print("User associated with provided Session ID had insufficient permissions to make a new directory/")
    else:
        print(f"Error received: Code {return_code}")
        if return_code == 0xFF:
            print("Operation failed or not implemented.")


def process_data(data):
    position = 0
    while position < len(data):
        file_type = data[position]
        position += 1
        file_name = b""
        while position < len(data) and data[position] != 0x00:
            file_name += bytes([data[position]])
            position += 1
        position += 1  # Skip the null terminator
        if file_type == 0x02:
            print(f"[DIR] {file_name.decode()}")
        else:
            print(f"[FILE] {file_name.decode()}")


def send_ls_request(sock, session_id, directory_name):
    opcode = 0x03
    directory_name = directory_name.encode() if directory_name else b''  # for blank /tmp
    dir_name_len = len(directory_name)

    request = struct.pack('>B B H I I', opcode, 0, dir_name_len, session_id, 0) + directory_name
    sock.sendall(request)

    print(f"Sending ls request for directory '{directory_name.decode()}' with session ID {session_id}.")
    receive_response(sock)

