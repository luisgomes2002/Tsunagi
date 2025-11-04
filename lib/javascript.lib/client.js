const Tsunagi = require("./tsunagi.lib");

(async () => {
  const client = new Tsunagi("ClientMultiRush");
  await client.connect();
  client.setLogging(true);

  client.onNotification((queueId, type) => {
    console.log(
      `🚨 Notificação recebida: a fila "${queueId}" expirou! (tipo: ${type})`,
    );
  });

  console.log("📦 Publicando mensagens nas diferentes filas...");

  // Fila 1 — tipo rushTimed
  await client.publish(
    "rushTimedQueue",
    "Mensagem RUSH_TIMED temporizada",
    15000,
    "rushTimed",
  );

  // Fila 2 — tipo single
  await client.publish(
    "singleQueue",
    "Mensagem SINGLE unitária",
    null,
    "single",
  );

  await client.publish(
    "rushTimedQueue 2",
    "Mensagem RUSH_TIMED temporizada 2",
    5000,
    "rushTimed",
  );

  await client.publish(
    "rushTimedQueue 3",
    "Mensagem RUSH_TIMED temporizada 3",
    10000,
    "rushTimed",
  );

  // Fila 3 — tipo rush
  await client.publish("rushQueue", "Mensagem RUSH normal", null, "rush");

  await client.single("singleQueue");
  await client.rush("rushQueue");

  console.log("✅ Mensagens publicadas. Aguardando expiração...");

  setInterval(() => {}, 1000);
})();
