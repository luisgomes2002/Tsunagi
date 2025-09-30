const Tsunagi = require("./tsunagi.lib");

(async () => {
  const client = new Tsunagi("Gym");

  try {
    await client.connect();
    client.setLogging(false);

    const messages = [
      { id: "1", payload: "Mensagem 1" },
      { id: "2", payload: "Mensagem 2" },
      { id: "3", payload: "Mensagem 3" },
      { id: "4", payload: "Mensagem 4" },
      { id: "5", payload: "Mensagem 5" },
      { id: "1", payload: "Mensagem 6" },
      { id: "1", payload: "Mensagem 7" },
      { id: "1", payload: "Mensagem 8" },
    ];

    for (const msg of messages) {
      await client.publish(msg.id, msg.payload);
    }

    let message = await client.rush("1");

    console.log("Mensagens: " + JSON.stringify(message, null, 2));

    console.log("Todas as mensagens enviadas!");

    client.disconnect();
  } catch (err) {
    console.error("Falha na conexão:", err);
  }
})();
