import socket
import numpy as np

frame_header_dt = np.dtype(
    [
        ("Frame Number", "u8"),
        ("SubFrame Number/ExpLength", "u4"),
        ("Packet Number", "u4"),
        ("Bunch ID", "u8"),
        ("Timestamp", "u8"),
        ("Module Id", "u2"),
        ("Row", "u2"),
        ("Column", "u2"),
        ("Reserved", "u2"),
        ("Debug", "u4"),
        ("Round Robin Number", "u2"),
        ("Detector Type", "u1"),
        ("Header Version", "u1"),
    ]
)


ip = "10.30.20.6"
ports = list(range(50200, 50204, 1))
sockets = [socket.socket(socket.AF_INET, socket.SOCK_DGRAM) for i in range(len(ports))]

for s, p in zip(sockets, ports):
    print("IP:Port: ", ip, p)
    s.bind((ip, p))

while True:
    for s in sockets:
        data, address = s.recvfrom(4096)
        h = np.frombuffer(data, count=1, dtype=frame_header_dt)[0]
        print(
            f'[{h["Timestamp"]}] frame: {h["Frame Number"]}, pkt:{h["Packet Number"]}, explength:{h["SubFrame Number/ExpLength"]}, module id: {h["Module Id"]} ,row: {h["Row"]}, column: {h["Column"]}'
        )