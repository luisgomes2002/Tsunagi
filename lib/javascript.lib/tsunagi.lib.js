import net from "net";

export default class Tsunagi {
  constructor(clientName, host = "127.0.0.1", port = 5000) {
    this.clientName = clientName;
    this.host = host;
    this.port = port;
    this.socket = new net.Socket();
    this.buffer = Buffer.alloc(0);
    this.logging = false;
    this.notificationCallback = null;
  }

  async connect() {
    return new Promise((resolve, reject) => {
      this.socket.connect(this.port, this.host, () => {
        this._sendName();
        if (this.logging) console.log(`🔌 Conectado como ${this.clientName}`);
        resolve();
      });

      this.socket.on("data", (data) => this._onData(data));
      this.socket.on("close", () => {
        if (this.logging) console.log("❌ Conexão encerrada pelo servidor");
      });
      this.socket.on("error", reject);
    });
  }

  setLogging(enabled) {
    this.logging = enabled;
  }

  onNotification(callback) {
    this.notificationCallback = callback;
  }

  // ---------------------------
  //  Métodos públicos
  // ---------------------------

  async publish(queue, payload, duration = 0, type = "rush") {
    const msg = `PUB|${queue}|${payload}|${duration}|${type}`;
    this._sendMessage(msg);
    if (this.logging)
      console.log(`📤 [PUB] -> ${queue} (${type}, ${duration}ms)`);
  }

  async rush(queue) {
    const msg = `RUSH|${queue}`;
    this._sendMessage(msg);
    return new Promise((resolve) => {
      this._waitForResponse("RUSH", queue, resolve);
    });
  }

  async single(queue) {
    const msg = `SINGLE|${queue}`;
    this._sendMessage(msg);
    return new Promise((resolve) => {
      this._waitForResponse("SINGLE", queue, resolve);
    });
  }

  // ---------------------------
  //  Internos
  // ---------------------------

  _sendName() {
    const nameBuf = Buffer.from(this.clientName);
    const nameSize = Buffer.alloc(4);
    nameSize.writeInt32LE(nameBuf.length);
    this.socket.write(Buffer.concat([nameSize, nameBuf]));
  }

  _sendMessage(msg) {
    const buf = Buffer.from(msg);
    const size = Buffer.alloc(4);
    size.writeInt32LE(buf.length);
    this.socket.write(Buffer.concat([size, buf]));
  }

  _onData(data) {
    this.buffer = Buffer.concat([this.buffer, data]);

    while (this.buffer.length >= 4) {
      const size = this.buffer.readInt32LE(0);
      if (this.buffer.length < 4 + size) break;

      const message = this.buffer.slice(4, 4 + size).toString();
      this.buffer = this.buffer.slice(4 + size);
      this._handleMessage(message);
    }
  }

  _handleMessage(message) {
    const parts = message.split("|", 3);
    const type = parts[0];
    const queueId = parts[1];
    const payload = parts[2];

    if (type === "EXPIRED") {
      if (this.logging)
        console.log(
          `🚨 Notificação recebida: a fila "${queueId}" expirou! (tipo: ${payload})`,
        );
      if (this.notificationCallback)
        this.notificationCallback(queueId, payload);
      return;
    }

    try {
      const json = JSON.parse(payload);
      if (
        this.pending &&
        this.pending.type === type &&
        this.pending.queue === queueId
      ) {
        this.pending.resolve(json);
        this.pending = null;
      } else if (this.logging) {
        console.log(`📩 Mensagem da fila '${queueId}':`, json);
      }
    } catch {
      if (this.logging)
        console.warn(`⚠️ Mensagem não é JSON válido: ${message}`);
    }
  }

  _waitForResponse(type, queue, resolve) {
    this.pending = { type, queue, resolve };
  }
}
