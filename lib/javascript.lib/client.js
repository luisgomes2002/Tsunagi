import Tsunagi from "./tsunagi.lib.js";

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
    JSON.stringify({
      tipo: "RUSH_TIMED",
      conteudo: "Mensagem temporizada",
      criadoEm: new Date().toISOString(),
    }),
    15000,
    "rushTimed",
  );

  // Fila 2 — tipo single
  await client.publish(
    "singleQueue",
    JSON.stringify({
      tipo: "SINGLE",
      conteudo: "Mensagem única",
      criadoEm: new Date().toISOString(),
    }),
    0,
    "single",
  );

  // Outras filas de exemplo
  await client.publish(
    "rushTimedQueue 2",
    JSON.stringify({
      tipo: "RUSH_TIMED",
      conteudo: "Mensagem temporizada 2",
      criadoEm: new Date().toISOString(),
    }),
    5000,
    "rushTimed",
  );

  await client.publish(
    "rushTimedQueue 3",
    JSON.stringify({
      tipo: "RUSH_TIMED",
      conteudo: "Mensagem temporizada 3",
      criadoEm: new Date().toISOString(),
    }),
    10000,
    "rushTimed",
  );

  // Fila 3 — tipo rush
  await client.publish(
    "rushQueue",
    JSON.stringify({
      tipo: "RUSH",
      conteudo: "Mensagem de rush normal",
      criadoEm: new Date().toISOString(),
    }),
    0,
    "rush",
  );

  const msg1 = await client.single("singleQueue");
  const msg2 = await client.rush("rushQueue");

  console.log("📩 Mensagem SINGLE recebida:", msg1);
  console.log("📩 Mensagem RUSH recebida:", msg2);

  console.log("✅ Mensagens publicadas. Aguardando expiração...");

  setInterval(() => {}, 1000);
})();
