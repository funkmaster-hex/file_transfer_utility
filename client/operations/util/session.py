Session_ID = 0

# Just a global getter and setter for holding the session ID to be used by the clients


def set_session_id(s):
    global Session_ID
    Session_ID = s
    return Session_ID


def get_session_id():
    return Session_ID


# This will clear the session id
def delete_session():
    global Session_ID
    Session_ID = 0

