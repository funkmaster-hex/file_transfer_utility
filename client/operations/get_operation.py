import struct
import os


def recv_all(socks, length, chunk_size=1024):
    data = b''
    while len(data) < length:
        to_receive = min(length - len(data), chunk_size)
        packet = socks.recv(to_receive)
        if not packet:
            return None  # Connection closed
        data += packet
    return data


def extract_valid_response(data):
    if len(data) >= 6:
        adjusted_response = data[3:] + data[:3]
        return adjusted_response
    return None


def send_get_request(sock, session_id, remote_file, local_file=None):
    if not local_file:
        local_file = os.path.basename(remote_file)

    # Prepare the GET request with the 0x04 opcode
    filepath_length = len(remote_file)
    request = struct.pack('>B B I H', 0x04, 0, session_id, filepath_length) + remote_file.encode()
    print(f"Request packed as: {request}")
    sock.sendall(request)
    print(f"Sent GET request for file '{remote_file}' with session ID {session_id}.")

    # Receive and process the header
    header = recv_all(sock, 8)  # Adjusted to 8 bytes to match header size
    if len(header) < 8:
        print("Error: Incomplete header received.")
        return

    return_code, = struct.unpack('>B', header[0:1])
    content_length, = struct.unpack('>I', header[2:6])
    print(f"Received header: return code {return_code}, content length {content_length}")

    if return_code == 0x01:
        print(f"File '{remote_file}' saved as '{local_file}'.")
    elif return_code == 0xFF:
        print(f"File {remote_file} cannot be found")
        return
    elif return_code == 0x02:
        print("Provided Session ID was invalid or expired")
        return
    elif return_code == 0x03:
        print("User associated with provided Session ID had insufficient permissions to read from a directory")
        return
    else:
        print("Unknown return code from server.")

    # Receive the file content in chunks of up to 1024 bytes
    received_data = recv_all(sock, content_length, chunk_size=1024)
    if received_data is None:
        print("Error: Connection closed before all data was received.")
        return

    if len(received_data) != content_length:
        print(
            f"Error: Received data length {len(received_data)} does not match expected "
            f"content length {content_length}.")
        return

    # Write the received content to the local file
    with open(local_file, 'wb') as f:
        f.write(received_data)

    print(f"File '{remote_file}' saved as '{local_file}'.")
