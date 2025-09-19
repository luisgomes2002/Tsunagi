const net = require("net");

class Tsunagi {
  constructor(clientName, host = "127.0.0.1", port = 5000) {
    this.clientName = clientName;
    this.host = host;
    this.port = port;
    this.client = new net.Socket();
    this.isConnected = false;
    this.logging = true;

    this.client.on("data", (data) => this.onData(data));
    this.client.on("close", () => this.onClose());
    this.client.on("error", (err) => this.onError(err));
  }

  setLogging(enable) {
    this.logging = enable;
  }

  log(...args) {
    if (this.logging) console.log(...args);
  }

  connect() {
    return new Promise((resolve) => {
      this.client.connect(this.port, this.host, () => {
        this.isConnected = true;
        this.log(
          `Conectado ao servidor como ${this.clientName} usando a porta ${this.port}`,
        );
        this.send(`${this.clientName}`);
        resolve();
      });
    });
  }

  send(message) {
    if (!this.isConnected) {
      if (this.logging) console.error("Cliente não está conectado!");
      return;
    }
    this.client.write(message);
  }

  async enqueueMessage(id, message, delay = 100) {
    const newMessage = `${id}|${message}`;
    this.send(newMessage);
    await new Promise((resolve) => setTimeout(resolve, delay));
  }

  onData(data) {
    this.log("Recebido do servidor:", data.toString());
  }

  onClose() {
    this.isConnected = false;
    this.log("Conexão encerrada");
  }

  onError(err) {
    if (this.logging) console.error("Erro:", err.message);
  }

  disconnect() {
    this.client.end();
  }
}

module.exports = Tsunagi;
