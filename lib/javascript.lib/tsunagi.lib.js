const net = require("net");

class Tsunagi {
  constructor(clientName) {
    this.clientName = clientName;
    this.host = "127.0.0.1";
    this.port = 5000;
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
    const msgBuffer = Buffer.from(message, "utf8");
    const sizeBuffer = Buffer.alloc(4);
    sizeBuffer.writeInt32LE(msgBuffer.length, 0);

    const canSend =
      this.client.write(sizeBuffer) && this.client.write(msgBuffer);

    if (!canSend) {
      return new Promise((resolve) => {
        this.client.once("drain", resolve);
      });
    }

    return Promise.resolve();
  }

  async publish(id, message, delay = 100) {
    const newMessage = `PUB|${id}|${message}`;
    await this.send(newMessage);

    if (delay > 0) {
      await new Promise((resolve) => setTimeout(resolve, delay));
    }
  }

  async rush(queueId) {
    const command = `CON|${queueId}`;
    await this.send(command);

    return new Promise((resolve) => {
      this.client.once("data", (data) => {
        const size = data.readInt32LE(0);
        const payload = data.slice(4, 4 + size).toString("utf8");

        if (payload === "EMPTY") {
          resolve([]);
        } else {
          const mensagens = payload.split("\n").filter((m) => m.length > 0);
          resolve(mensagens);
        }
      });
    });
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
