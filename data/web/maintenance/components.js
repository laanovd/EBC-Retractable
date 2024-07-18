// Indicator
function INIT_indicator(
  elm_id,
  json_key,
  disabled_color = "bg-slate-300",
  enabled_color = "bg-emerald-500"
) {
  const elm = document.querySelector(`#${elm_id}`);
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      const value = JSON.parse(data[json_key]);
      elm.classList.add(value ? enabled_color : disabled_color);
      elm.classList.remove(value ? disabled_color : enabled_color);
    }
  });
}

// Toggle
function INIT_toggle(elm_id, json_key) {
  const elm = document.querySelector(`#${elm_id}`);
  elm.addEventListener("click", (e) => {
    e.preventDefault();
    e.stopPropagation();
    elm.disabled = true;
    sendCommand(JSON.stringify({ [json_key]: e.target.checked }));
    setTimeout(() => {
      elm.disabled = false;
    }, 5000);
  });
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      console.log(`${json_key}: ${data[json_key]}`);
      elm.checked = JSON.parse(data[json_key]);
      elm.disabled = false;
    }
  });
}

// Number input
let values = {};
function INIT_number_input(elm_id, json_key) {
  values[json_key] = 0;
  const elm = document.querySelector(`#${elm_id}`);
  elm.addEventListener("change", (e) => {
    const val = e.target.value;
    elm.value = values[json_key];
    sendCommand(JSON.stringify({ [json_key_azimuth_offset_right]: val }));
  });
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key)) {
      elm.value = JSON.parse(data[json_key]);
    }
  });
}

// Steering
const elm_id_steering = "steering_manual";
const json_key_steering = "steering_manual";
let steering_manual_current = 0;
function INIT_steering() {
  const elm = document.querySelector(`#${elm_id_steering}`);
  let timeout;
  elm.addEventListener("change", (e) => {
    timeout = setTimeout(() => {
      elm.value = steering_manual_current;
    }, 1000)
    sendCommand(JSON.stringify({ [json_key_steering]: e.target.value }));
  });
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key_steering)) {
      clearTimeout(timeout);
      steering_manual_current = JSON.parse(data[json_key_steering]);
      elm.value = steering_manual_current;
    }
  });
}
// Init
document.addEventListener(
  "DOMContentLoaded",
  function () {
    INIT_toggle("maintenance_enabled", "maintenance_enabled");
    INIT_toggle("lift_homing", "lift_homing");
    INIT_toggle("lift_enabled", "lift_enabled");
    INIT_toggle("retract_enabled", "lift_motor_up");
    INIT_toggle("extend_enabled", "lift_motor_down");
    INIT_toggle("dmc_enabled", "dmc_enabled");
    INIT_toggle("azimuth_homing", "steering_homing");
    INIT_toggle("steering_enabled", "steering_enabled");
    INIT_toggle("output_enabled", "steering_analog_out_enabled"); // TODO

    INIT_indicator(
      "emergency_stop_indicator",
      "emergency_stop",
      "bg-emerald-500",
      "bg-rose-500"
    );
    INIT_indicator("retracted_indicator", "lift_sensor_up");
    INIT_indicator("extended_indicator", "lift_sensor_down");

    INIT_number_input("azimuth_left", "steering_left_volt");
    INIT_number_input("azimuth_right", "steering_right_volt");
    INIT_number_input("azimuth_actual", "no_key"); // TODO
    INIT_number_input("azimuth_timeout", "steering_delay_to_the_middle");
    INIT_number_input("wheel_left", "no_key"); // TODO
    INIT_number_input("wheel_right", "no_key"); // TODO
    INIT_number_input("wheel_middle", "no_key"); // TODO

    INIT_steering();

    addSingleMessageHandler((data) => {
      document.querySelector("#loading_overlay").classList.add("hidden");
    });
  },
  false
);
