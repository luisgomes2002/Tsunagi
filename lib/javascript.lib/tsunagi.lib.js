const net = require("net");

class Tsunagi {
  constructor(clientName, host = "127.0.0.1", port = 5000) {
    this.clientName = clientName;
    this.host = host;
    this.port = port;
    this.client = new net.Socket();
    this.isConnected = false;
    this.logging = true;

    // Handlers e buffers
    this.notificationHandlers = [];
    this.buffer = Buffer.alloc(0);
    this.pendingResolvers = []; // fila de Promises esperando resposta

    // Eventos
    this.client.on("data", (data) => this.onData(data));
    this.client.on("close", () => this.onClose());
    this.client.on("error", (err) => this.onError(err));
  }

  // =====================================================
  // ⚙️ Conexão e utilitários
  // =====================================================

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
        this.send(this.clientName);
        resolve();
      });
    });
  }

  send(message) {
    const msgBuffer = Buffer.from(message, "utf8");
    const sizeBuffer = Buffer.alloc(4);
    sizeBuffer.writeInt32LE(msgBuffer.length, 0);
    this.client.write(sizeBuffer);
    this.client.write(msgBuffer);
  }

  disconnect() {
    this.client.end();
  }

  // =====================================================
  // 📨 Publicação
  // =====================================================

  async publish(queueId, message, duration = null, type = "rush", delay = 100) {
    const formatted = duration
      ? `PUB|${queueId}|${message}|${duration}|${type}`
      : `PUB|${queueId}|${message}|0|${type}`;
    this.send(formatted);
    if (delay > 0) await new Promise((r) => setTimeout(r, delay));
  }

  // =====================================================
  // 🧺 Consumo com Promises controladas
  // =====================================================

  _sendAndWaitResponse(command) {
    this.send(command);
    return new Promise((resolve) => {
      this.pendingResolvers.push(resolve);
      // opcional: timeout pode ser adicionado aqui
    });
  }

  async rushTimed(queueId) {
    const payload = await this._sendAndWaitResponse(`RUSH_TIMED|${queueId}`);
    return payload === "EMPTY" ? null : payload;
  }

  async rush(queueId) {
    const payload = await this._sendAndWaitResponse(`RUSH|${queueId}`);
    if (payload && payload !== "EMPTY")
      console.log(`⚡ [RUSH] Consumido: ${payload}`);
    else console.log(`⚠️ Fila "${queueId}" vazia (rush)`);
    return payload === "EMPTY" ? null : payload;
  }

  async single(queueId) {
    const payload = await this._sendAndWaitResponse(`SINGLE|${queueId}`);
    if (payload && payload !== "EMPTY")
      console.log(`🎯 [SINGLE] Consumido: ${payload}`);
    else console.log(`⚠️ Fila "${queueId}" vazia (single)`);
    return payload === "EMPTY" ? null : payload;
  }

  // =====================================================
  // 🔔 Sistema de Notificações
  // =====================================================

  onNotification(handler) {
    this.notificationHandlers.push(handler);
  }

  async handleNotification(queueId, queueType) {
    this.log(`⚠️ Fila expirada detectada: "${queueId}" (tipo: ${queueType})`);

    if (queueType === "rushTimed") {
      // ✅ Apenas rushTimed é consumida automaticamente
      const result = await this.rushTimed(queueId);
      console.log(
        result
          ? `🕒 [RUSH_TIMED] Consumido automaticamente: ${result}`
          : `❌ Nenhum item disponível em "${queueId}"`,
      );
    }
  }

  // =====================================================
  // 📡 Fragmentação TCP e parser unificado
  // =====================================================

  onData(data) {
    this.buffer = Buffer.concat([this.buffer, data]);

    while (this.buffer.length >= 4) {
      const size = this.buffer.readInt32LE(0);
      if (this.buffer.length < size + 4) break;

      const payload = this.buffer.slice(4, 4 + size).toString("utf8");
      this.buffer = this.buffer.slice(4 + size);

      if (payload.startsWith("EXPIRED|")) {
        const [, queueId, queueType] = payload.split("|");

        // notifica listeners
        this.notificationHandlers.forEach((fn) => fn(queueId, queueType));

        // consome automaticamente
        this.handleNotification(queueId, queueType);
      } else {
        // resposta a RUSH/SINGLE
        if (this.pendingResolvers.length > 0) {
          const resolve = this.pendingResolvers.shift();
          resolve(payload);
        } else {
          this.log("Mensagem inesperada do servidor:", payload);
        }
      }
    }
  }

  onClose() {
    this.log("🔌 Conexão encerrada com o servidor.");
    this.isConnected = false;
  }

  onError(err) {
    console.error("Erro:", err.message);
  }
}

module.exports = Tsunagi;
