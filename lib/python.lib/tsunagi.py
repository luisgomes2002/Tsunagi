import socket
import time

HOST = '127.0.0.1'  
PORT = 5000         

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    client_id = "Murasaki"
    s.sendall(client_id.encode())
    print(f"Nome enviado: {client_id}\n")

    time.sleep(1)

    messages_plan = [
        ("2", 5),  
        ("1", 2), 
        ("5", 3),  
        ("7", 4), 
    ]

    count = 1
    try:
        for msg_id, qty in messages_plan:
            for _ in range(qty):
                payload = f"Mensagem {count}"
                mensagem_com_id = f"{msg_id}|{payload}"
                s.sendall(mensagem_com_id.encode())
                print(f"Enviado: {mensagem_com_id}")
                count += 1
                time.sleep(0.05)  
    except KeyboardInterrupt:
        print("\nCliente encerrado pelo usuário.")
