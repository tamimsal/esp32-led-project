const express = require('express');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

const TOKEN = process.env.TOKEN || 'supersecret';
const commands = new Map();  // deviceId -> "ON" | "OFF"

// Health check
app.get('/api/health', (req, res) => res.send('ok'));

// Web page posts a command
app.post('/api/cmd', (req, res) => {
  const { device, command, key } = req.body || {};
  if (key !== TOKEN) return res.status(401).json({ error: 'unauthorized' });
  const cmd = String(command || '').toUpperCase();
  if (!device || !['ON', 'OFF'].includes(cmd))
    return res.status(400).json({ error: 'bad request' });

  commands.set(device, cmd);
  res.json({ status: 'queued', device, command: cmd });
});

// ESP32 polls commands
app.get('/api/cmd', (req, res) => {
  const { device, key } = req.query;
  if (key !== TOKEN) return res.status(401).send('UNAUTHORIZED');
  if (!device) return res.status(400).send('BAD_REQUEST');
  const cmd = commands.get(device) || 'NONE';
  commands.delete(device);
  res.type('text/plain').send(cmd);
});

const port = process.env.PORT || 3000;
app.listen(port, () => console.log('Server listening on port ' + port));
