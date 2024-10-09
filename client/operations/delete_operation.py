import struct

DELETE_OPCODE = 0x02  # Define the DELETE_OPCODE - dumb, but maybe to change easier later.


# Here we will wait to receive all packets first based on the length
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


def send_delete_request(sock, session_id, filename):
    opcode = DELETE_OPCODE
    reserved = 0x00
    filename_bytes = filename.encode('utf-8')
    filename_length = len(filename_bytes)

    session_id_bytes = struct.pack("!I", session_id)
    print(session_id)

    # Pack the opcode, reserved byte, filename length, session ID, and filename together
    request = struct.pack(f"!BBH", opcode, reserved, filename_length) + session_id_bytes + filename_bytes

    # Debugging: print the request packet
    print(f"Sending request: {request}")

    # Send the request
    sock.sendall(request)

    # Receive the response
    response_code = recv_all(sock, 8)
    print(f"Response code: {response_code}")

    if response_code and len(response_code) >= 6:

        return_code, = struct.unpack("!B", response_code[:1])
        if return_code == 0x01:
            print(f"File '{filename}' deleted successfully.")
        elif return_code == 0xff:
            print(f"Failed to delete file '{filename}'.")
        elif return_code == 0x02:
            print("Provided Session ID was invalid or expired.")
        elif return_code == 0x03:
            print(f"Insufficient permission to delete file: {filename}.")
        else:
            print("Unknown response from server.")
