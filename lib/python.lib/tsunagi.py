import socket
import time

HOST = '127.0.0.1'  
PORT = 5000         

client_id = "client1"

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    
    for i in range(5):
        mensagem = f"{client_id}|Mensagem {i+1}"
        s.sendall(mensagem.encode())
        print(f"Enviado: {mensagem}")
        
        if i < 4: 
            time.sleep(1)
