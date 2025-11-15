import Tsunagi from "./tsunagi.lib.js";

function wait(ms) {
  return new Promise((r) => setTimeout(r, ms));
}

async function simulateEmailRush() {
  const client = new Tsunagi("EmailAggregator");
  await client.connect();
  client.setLogging(true);

  const inicio = performance.now();

  console.log("🚀 Iniciando simulação de carga...");

  const todas = [];
  let totalEnviado = 0;
  let totalRecebido = 0;

  const TOTAL = 5000;
  const TOTAL_USERS = 1600;

  for (let i = 0; i < TOTAL; i++) {
    const userId = Math.floor(Math.random() * TOTAL_USERS) + 1;

    const msg = {
      id: i,
      userId,
      titulo: `Atualização #${i}`,
      conteudo: `Mensagem #${i} do usuário ${userId}.`,
      prioridade: i % 5 === 0 ? "alta" : "normal",
      criadoEm: new Date().toISOString(),
    };

    todas.push(msg);
    totalEnviado++;

    await client.publish("rushQueue", JSON.stringify(msg), 0, "rush");

    if (i % 500 === 0) {
      console.log(`📤 Enviadas ${i}/${TOTAL} mensagens...`);
    }

    await wait(3);
  }

  totalRecebido = todas.length;

  console.log(`📦 Todas as ${TOTAL} mensagens foram publicadas!`);

  const payloadGigante = { messages: todas };

  gerarEmail(payloadGigante);

  const fim = performance.now();
  const tempoSeg = (fim - inicio) / 1000;

  console.log("\n====== MÉTRICAS FINAIS ======");
  console.log(`⏳ Tempo rodando: ${tempoSeg.toFixed(2)}s`);
  console.log(`📥 Total recebido: ${totalRecebido}`);
  console.log(`📤 Total enviado: ${totalEnviado}`);
  console.log(
    `🚀 Velocidade de recebimento: ${(totalRecebido / tempoSeg).toFixed(
      2,
    )} msg/s`,
  );
  console.log(
    `📧 Velocidade de envio: ${(totalEnviado / tempoSeg).toFixed(2)} msg/s`,
  );
  console.log("================================\n");
}

function gerarEmail(data) {
  const msgs = data.messages;
  const total = msgs.length;

  const preview = msgs.slice(0, 3).map((m, i) => {
    return `(${i + 1}) [User ${m.userId}] ${m.titulo} — ${m.conteudo}`;
  });

  const email = `
📪 VOCÊ TEM ${total} NOVAS ATUALIZAÇÕES

🔎 PRIMEIRAS 3 MENSAGENS:
${preview.join("\n")}
--------------------------------
• Prioridade alta: ${msgs.filter((m) => m.prioridade === "alta").length}
• Prioridade normal: ${msgs.filter((m) => m.prioridade === "normal").length}
• Última criada: ${msgs[msgs.length - 1].criadoEm}
--------------------------------
`.trim();

  console.log("\n===== EMAIL GERADO =====");
  console.log(email);
  console.log("========================\n");
}

simulateEmailRush();
