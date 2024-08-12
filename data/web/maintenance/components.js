const json_key_maintenance = "maintenance_enabled";
let maintenance_enabled;
function INIT_maintenance() {
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key_maintenance)) {
      const inputs = document.querySelectorAll("input.mt_only");
      maintenance_enabled = JSON.parse(data[json_key_maintenance]);
      inputs.forEach((elm) => {
        elm.disabled = !maintenance_enabled;
      });
    }
  });
}

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
      if (maintenance_enabled || elm.id == json_key_maintenance)
        elm.disabled = false;
    }
  });
}

// Number input
let values = {};
function INIT_number_input(elm_id, json_key) {
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
      elm.value = JSON.parse(data[json_key]);
    }
  });
}

// Steering
const elm_id_steering = "azimuth_manual";
const json_key_steering = "azimuth_manual";
let steering_manual_current = 0;
function INIT_steering() {
  const elm = document.querySelector(`#${elm_id_steering}`);
  let timeout;
  elm.addEventListener("change", (e) => {
    timeout = setTimeout(() => {
      elm.value = steering_manual_current;
    }, 2000);
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

// Maintenance button
function INIT_maintenance_button() {
  const elm = document.querySelector(`#maintenance_enabled`);
  elm.addEventListener("click", (e) => {
    e.preventDefault();
    e.stopPropagation();
    elm.disabled = true;
    if (e.target.checked) showMaintenanceEnable(true);
    else sendCommand(JSON.stringify({ maintenance_enabled: false }));
  });
  addMessageHandler((data) => {
    if (data.hasOwnProperty(json_key_maintenance)) {
      elm.checked = JSON.parse(data[json_key_maintenance]);
      elm.disabled = false;
    }
  });
}

// Init
document.addEventListener(
  "DOMContentLoaded",
  function () {
    INIT_maintenance();

    INIT_maintenance_button();
    INIT_toggle("lift_homing", "lift_homing");
    INIT_toggle("lift_enabled", "lift_enabled");
    INIT_toggle("retract_enabled", "lift_motor_up");
    INIT_toggle("extend_enabled", "lift_motor_down");
    INIT_toggle("dmc_enabled", "dmc_enabled");

    INIT_number_input("azimuth_low", "azimuth_low_counts");
    INIT_number_input("azimuth_middle", "azimuth_middle_counts");
    INIT_number_input("azimuth_high", "azimuth_high_counts");
    INIT_number_input("azimuth_actual", "azimuth_actual_counts");
    INIT_number_input("azimuth_timeout", "azimuth_timeout_to_the_middle");
    INIT_toggle("azimuth_homing", "azimuth_homing");
    INIT_toggle("azimuth_enabled", "azimuth_enabled");
    INIT_toggle("output_enabled", "azimuth_analog_out_enabled");
    INIT_toggle("save_steeringwheel_calibration", "save_steeringwheel_calibration");
    INIT_toggle("restore_steeringwheel_calibration", "restore_steeringwheel_calibration");
    INIT_toggle("save_azimuth_calibration", "save_azimuth_calibration");
    INIT_toggle("restore_azimuth_calibration", "restore_azimuth_calibration");

    INIT_indicator(
      "emergency_stop_indicator",
      "emergency_stop",
      "bg-emerald-500",
      "bg-rose-500"
    );
    INIT_indicator("retracted_indicator", "lift_sensor_up");
    INIT_indicator("extended_indicator", "lift_sensor_down");
    INIT_indicator("azimuth_home", "azimuth_home");

    INIT_number_input("steering_left", "steering_left_counts");
    INIT_number_input("stering_right", "steering_right_counts");
    INIT_number_input("steering_middle", "steering_middle_counts");
    INIT_number_input("steering_actual", "steering_actual_counts");

    INIT_steering();
  },
  false
);

window.addEventListener("beforeunload", function (e) {
  sendCommand(JSON.stringify({ maintenance_enabled: false }));
});
