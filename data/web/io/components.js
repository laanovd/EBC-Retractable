const digital_input_timeout = 5000; // (ms)

function INIT_digital_input(elm_id, json_key) {
  const elm = document.querySelector(`#${elm_id}`);
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      const value = JSON.parse(data[json_key]);
      elm.ariaChecked = value;
    }
  });
}

function INIT_digital_output(elm_id, json_key) {
  const elm = document.querySelector(`#${elm_id}`);
  let timeout;
  elm.addEventListener("click", (e) => {
    e.preventDefault();
    e.stopPropagation();
    elm.disabled = true;
    sendCommand(JSON.stringify({ [json_key]: e.target.checked ? true : false }));
    timeout = setTimeout(() => {
      elm.disabled = false;
    }, digital_input_timeout);
  });
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      const value = JSON.parse(data[json_key]);
      elm.ariaChecked = value;
      elm.disabled = false;
      clearTimeout(timeout)
    }
  });
}

function INIT_analog_input(elm_id, json_key) {
  const elm = document.querySelector(`#${elm_id}`);
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      const value = JSON.parse(data[json_key]);
      elm.value = value; 
    }
  });
}

let values = {};
function INIT_analog_output(elm_id, json_key) {
  values[json_key] = 0;
  const elm = document.querySelector(`#${elm_id}`);
  let timeout;
  elm.addEventListener("change", (e) => {
    const val = e.target.value;
    timeout = setTimeout(() => {
      elm.value = values[json_key];
    }, 2000);
    sendCommand(JSON.stringify({ [json_key]: val }));
  });
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      clearTimeout(timeout);
      const val = JSON.parse(data[json_ke])
      elm.value = val;
      values[json_key] = val;
    }
  });
}

// Init
document.addEventListener("DOMContentLoaded", function () {
  INIT_digital_input("input1", "input1");
  INIT_digital_input("input2", "input2");
  INIT_digital_input("input3", "input3");
  INIT_digital_input("input4", "input4");
  INIT_digital_input("input5", "input5");
  INIT_digital_input("input6", "input6");

  INIT_digital_output("output1", "output1");
  INIT_digital_output("output2", "output2");
  INIT_digital_output("output3", "output3");
  INIT_digital_output("output4", "output4");
  INIT_digital_output("output5", "output5");
  INIT_digital_output("output6", "output6");
  INIT_digital_output("output7", "output7");
  INIT_digital_output("output8", "output8");
  INIT_digital_output("output9", "output9");
  INIT_digital_output("output10", "output10");
  INIT_digital_output("output11", "output11");

  INIT_analog_input("analogin1", "analogin1");
  INIT_analog_input("analogin2", "analogin2");

  INIT_analog_output("analogout1", "analogout1");
  INIT_analog_output("analogout2", "analogout2");
}, false);
