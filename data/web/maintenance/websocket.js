// const socket = new WebSocket(`ws://192.168.1.133:81`);
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

function showMaintenanceEnable(value = true) {
  const elm = document.querySelector("#login_overlay");
  if (value) elm.classList.remove("hidden");
  else elm.classList.add("hidden");
}

function enableMaintenance() {
  const elm = document.querySelector("#password");
  if (elm.value == "ebce6671") {
    showMaintenanceEnable(false);
    sendCommand(JSON.stringify({ maintenance_enabled: true }));
  } else {
    elm.classList.add("border", "border-rose-500");
  }
}
