# import socket

# server = socket.socket()
# server.bind(('127.0.0.1', 12345))
# server.listen(1)
# print("Waiting for connection...")

# conn, addr = server.accept()
# print(f"Connected from {addr}")

# while True:
#     conn.send(b"hello from server\n")
#     data = conn.recv(1024)
#     if not data:
#         break
#     print("OBS sent:", data.decode())


import socket
import threading

def handle_client(conn, addr):
    print(f"[+] Connected by {addr}")
    try:
        while True:
            msg = input("Enter command to send (e.g. switch_scene:Intro): ")
            if not msg:
                continue
            conn.sendall(msg.encode('utf-8'))
    except (ConnectionResetError, BrokenPipeError):
        print("[-] Connection closed.")
    finally:
        conn.close()

def start_server():
    host = "127.0.0.1"
    port = 12345

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((host, port))
    server.listen(1)
    print(f"[+] Listening on {host}:{port}...")

    conn, addr = server.accept()
    client_thread = threading.Thread(target=handle_client, args=(conn, addr))
    client_thread.start()

if __name__ == "__main__":
    start_server()
