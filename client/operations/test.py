import struct

# Example response
response = b'\xc6\x00\x00\x01\x00\x00\x00\x00'

# Extract and adjust the response data
# We want to take the first 4 bytes and append them to the end of the response
adjusted_response = response[3:] + response[:3]

# Print the adjusted response for debugging
print(f"Adjusted response: {adjusted_response}")

# Unpack the adjusted response
# Ensure it has enough length to unpack
if len(adjusted_response) >= 5:
    # Unpack the return_code from the first byte
    return_code, = struct.unpack("!B", adjusted_response[:1])

    # Unpack the content_length from the next 4 bytes
    content_length, = struct.unpack("!I", adjusted_response[1:5])

    print(f"Return code: {return_code:#x}")
    print(f"Content length: {content_length}")
else:
    print("Received data is too short to unpack.")
