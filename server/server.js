const express = require('express');
const cors = require('cors');
const http = require('http');
const WebSocket = require('ws');

const app = express();
app.use(cors());
app.use(express.json());

const TOKEN = process.env.TOKEN || 'supersecret';

// Maps to track device state
const sockets = new Map();   // deviceId -> ws
const commands = new Map();  // deviceId -> "ON" | "OFF" (latest pending if device offline)

// Simple health check
app.get('/api/health', (req, res) => res.send('ok'));

// Frontend posts a command
app.post('/api/cmd', (req, res) => {
  const { device, command, key } = req.body || {};
  const cmd = String(command || '').toUpperCase();

  if (key !== TOKEN) return res.status(401).json({ error: 'unauthorized' });
  if (!device || !['ON', 'OFF'].includes(cmd))
    return res.status(400).json({ error: 'bad request' });

  const ws = sockets.get(device);

  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(cmd);                // push immediately to ESP32
    return res.json({ status: 'sent', device, command: cmd, delivered: true });
  } else {
    // Device offline: remember the latest command and deliver on next connect
    commands.set(device, cmd);
    return res.json({ status: 'queued', device, command: cmd, delivered: false });
  }
});

// (Optional legacy) If you want, keep GET for debugging; otherwise remove.
// We’ll explicitly tell clients not to use it anymore.
app.get('/api/cmd', (req, res) => res.status(410).send('Use WebSocket'));

const server = http.createServer(app);

// WebSocket server at ws(s)://<host>/ws
const wss = new WebSocket.Server({ server, path: '/ws' });

wss.on('connection', (ws, req) => {
  // Expect device and key in the querystring: /ws?device=esp32-1&key=supersecret
  const url = new URL(req.url, `http://${req.headers.host}`);
  const device = url.searchParams.get('device');
  const key = url.searchParams.get('key');

  if (!device || key !== TOKEN) {
    ws.close(1008, 'unauthorized');
    return;
  }

  // Track the socket
  sockets.set(device, ws);
  console.log('WS connected:', device);

  // If a command was queued while offline, deliver it now
  const pending = commands.get(device);
  if (pending) {
    ws.send(pending);
    commands.delete(device);
  }

  ws.on('message', (msg) => {
    console.log(`device[${device}] ->`, String(msg));
  });

  ws.on('close', () => {
    // Only delete if it's the same socket instance
    if (sockets.get(device) === ws) sockets.delete(device);
    console.log('WS closed:', device);
  });
});

const port = process.env.PORT || 3000;
server.listen(port, () => console.log('Server listening on port ' + port));
