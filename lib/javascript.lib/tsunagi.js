const net = require("net");

const HOST = "127.0.0.1";
const PORT = 5000;

const client = new net.Socket();

client.connect(PORT, HOST, () => {
  console.log("Conectado ao servidor");

  const clientId = "Gym";
  client.write(clientId);
  console.log(`Nome enviado: ${clientId}\n`);

  const messagesPlan = [
    ["2", 5],
    ["1", 2],
    ["5", 3],
    ["7", 4],
  ];

  let count = 1;

  const sendMessages = async () => {
    for (const [msgId, qty] of messagesPlan) {
      for (let i = 0; i < qty; i++) {
        const payload = `Mensagem ${count}`;
        const mensagemComId = `${msgId}|${payload}`;
        client.write(mensagemComId);
        console.log(`Enviado: ${mensagemComId}`);
        count++;
        await new Promise((resolve) => setTimeout(resolve, 3000));
      }
    }
  };

  sendMessages().catch(console.error);
});

client.on("data", (data) => {
  console.log("Recebido do servidor:", data.toString());
});

client.on("close", () => {
  console.log("Conexão encerrada");
});

client.on("error", (err) => {
  console.error("Erro:", err.message);
});
