const inbox = [];
let totalReceived = 0;
let totalSent = 0;

const totalToGenerate = 5000;
const TOTAL_USERS = 1600;

let startTime = performance.now();

// batches de chegada
const BATCH_SIZE = 10;

// workers enviando emails
const WORKERS = 3;
const SEND_INTERVAL = 200;

function fakeEmailSend(message) {
  return new Promise((resolve) => {
    setTimeout(() => {
      console.log(
        `📧 Enviado: #${message.id} (${message.user}) → "${message.text}"`,
      );
      resolve();
    }, 300);
  });
}

// WORKERS concorrentes
for (let w = 1; w <= WORKERS; w++) {
  setInterval(async () => {
    if (inbox.length === 0) return;

    const msg = inbox.shift();
    await fakeEmailSend(msg);

    totalSent++;

    if (totalSent === totalToGenerate && inbox.length === 0) {
      finalize();
    }
  }, SEND_INTERVAL);
}

// geração das mensagens
async function simulateMessages() {
  let counter = 1;

  while (counter <= totalToGenerate) {
    const amount = Math.min(BATCH_SIZE, totalToGenerate - counter + 1);

    console.log(`\n👥 Gerando batch de ${amount} mensagens...`);

    for (let i = 0; i < amount; i++) {
      const id = counter++;
      const msg = {
        id,
        text: `Mensagem simulada #${id}`,
        ts: new Date().toISOString(),
        user: `user_${Math.ceil(Math.random() * TOTAL_USERS)}`,
      };

      inbox.push(msg);
      totalReceived++;

      console.log(`📥 Recebida: #${msg.id} (de ${msg.user})`);
    }

    await new Promise((r) => setTimeout(r, 300 + Math.random() * 400));
  }

  console.log("\n🟢 Todas as mensagens foram geradas!\n");
}

// métricas finais
function finalize() {
  console.log("\n\n====== MÉTRICAS FINAIS ======");

  const now = performance.now();
  const runningSeconds = ((now - startTime) / 1000).toFixed(2);

  console.log(`⏳ Tempo rodando: ${runningSeconds}s`);
  console.log(`📥 Total recebido: ${totalReceived}`);
  console.log(`📤 Total enviado: ${totalSent}`);
  console.log(
    `🚀 Velocidade de recebimento: ${(totalReceived / runningSeconds).toFixed(
      2,
    )} msg/s`,
  );
  console.log(
    `📧 Velocidade de envio: ${(totalSent / runningSeconds).toFixed(2)} msg/s`,
  );

  console.log("================================\n");

  process.exit(0);
}

process.on("SIGINT", finalize);

simulateMessages();
