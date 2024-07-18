// const socket = new WebSocket(`ws://192.168.1.125:81`);
const socket = new WebSocket(`ws://${window.location.host}:81`);

const data = {};
const on_data = [];
const on_next_data = [];

socket.addEventListener("open", (event) => {
  document.querySelector("#loading_overlay").classList.add("hidden");
});
socket.addEventListener("close", (event) => {
  document.querySelector("#loading_overlay").classList.remove("hidden");
});

socket.addEventListener("message", (event) => {
  const socket_data = JSON.parse(event.data);
  on_data.forEach((fn) => fn(socket_data));
  on_next_data.forEach((fn) => fn(socket_data));
  on_next_data.length = 0;
});

function sendCommand(data) {
  socket.send(data);
  console.log(data);
}

function addMessageHandler(fn) {
  on_data.push(fn);
}

function addSingleMessageHandler(fn) {
  on_next_data.push(fn);
}
