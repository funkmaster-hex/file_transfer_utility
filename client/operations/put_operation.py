import struct


def recv_all(sock, length):
    data = b''
    while len(data) < length:
        packet = sock.recv(length - len(data))
        if not packet:
            return None  # Connection closed
        data += packet
    return data


def extract_valid_response(data):
    if len(data) >= 6:
        adjusted_response = data[3:] + data[:3]
        return adjusted_response
    return None


def send_put_request(sock, session_id, local_file, remote_file, overwrite_flag):
    # Read the content of the local file
    try:
        with open(local_file, 'rb') as f:
            file_content = f.read()
    except FileNotFoundError:
        print(f"Error: Local file '{local_file}' not found.")
        return

    file_content_length = len(file_content)
    filename_length = len(remote_file)

    overwrite_flag_byte = b'\x01' if overwrite_flag else b'\x00'

    # Pack the request - again this worked on its own in another setting
    # before I chopped it to hell to work in this setting.
    # '>': big-endian
    # B: unsigned char (1 byte)
    # H: unsigned short (2 bytes)
    # I: unsigned int (4 bytes)
    # {filename_length}s: string of length `filename_length`
    # {file_content_length}s: string of length `file_content_length`
    request_format = f'>B B H I I {filename_length}s {file_content_length}s'
    request = struct.pack(request_format,
                          0x06,
                          ord(overwrite_flag_byte),
                          filename_length,
                          session_id,
                          file_content_length,
                          remote_file.encode(),
                          file_content)

    # Send the request
    sock.sendall(request)
    print(f"Sent PUT request for file '{local_file}' to be saved as '{remote_file}' with session ID {session_id}.")

    # Receive the response code from the server
    response_code = recv_all(sock, 8)
    print(f"Response code: {response_code}")

    if response_code and len(response_code) >= 6:

        return_code, = struct.unpack("!B", response_code[:1])

        if return_code:
            if return_code == 0x01:
                print(f"File '{remote_file}' uploaded successfully.")
            elif return_code == 0x05:
                print(f"File '{remote_file}' already exists and cannot be overwritten.")
            elif return_code == 0xFF:
                print(f"Failed to upload file.")
            elif return_code == 0x02:
                print("Provided Session ID was invalid or expired")
            elif return_code == 0x03:
                print("User associated with provided Session ID had insufficient permissions to make a new directory")
            else:
                print("Response not implemented.")
        else:
            print("No response received from server.")
