// const socket = new WebSocket(`ws://192.168.1.123:81`);
const socket = new WebSocket(`ws://${window.location.host}:81`);

const data = {};
const on_data = [];
const on_next_data = [];

socket.addEventListener("message", (event) => {
  const socket_data = JSON.parse(event.data);
  on_data.forEach((fn) => fn(socket_data));
  on_next_data.forEach((fn) => fn(socket_data));
  on_next_data.length = 0;
});

function sendCommand(data) {
  socket.send(data);
}

function addMessageHandler(fn) {
  on_data.push(fn);
}
