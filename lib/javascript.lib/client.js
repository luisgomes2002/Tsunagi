const Tsunagi = require("./tsunagi.lib");

(async () => {
  const client = new Tsunagi("Gym");

  try {
    await client.connect();
    client.setLogging(false);

    // const messages = [
    //   { id: "2", payload: "Mensagem 1" },
    //   { id: "2", payload: "Mensagem 2" },
    //   { id: "1", payload: "Mensagem 3" },
    //   { id: "5", payload: "Mensagem 4" },
    //   { id: "7", payload: "Mensagem 5" },
    // ];

    const messages = [];

    const ids = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"];
    const totalMessages = 1000;

    for (let i = 0; i < totalMessages; i++) {
      const randomId = ids[Math.floor(Math.random() * ids.length)];
      const payload = `Mensagem ${i + 1}`;
      messages.push({ id: randomId, payload });
    }

    for (const msg of messages) {
      await client.enqueueMessage(msg.id, msg.payload);
    }

    console.log("Todas as mensagens enviadas!");

    client.disconnect();
  } catch (err) {
    console.error("Falha na conexão:", err);
  }
})();
