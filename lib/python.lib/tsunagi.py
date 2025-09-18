import socket
import time

HOST = '127.0.0.1'  
PORT = 5000         

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    client_id = "Murasaki"
    s.sendall(client_id.encode())
    print(f"Nome enviado: {client_id}")

    time.sleep(2)

    count = 1
    try:
        while True:
            mensagem = f"Mensagem {count}"
            s.sendall(mensagem.encode())
            print(f"Enviado: {mensagem}")
            count += 1
            time.sleep(2)  
    except KeyboardInterrupt:
        print("\nCliente encerrado pelo usuário.")
