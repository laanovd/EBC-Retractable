// Load data
const data_static = {
    "maintenance_active": false,
    "emergency_stop": false,
    "lift_enabled": false,
    "lift_motor_up": false,
    "lift_motor_down": true,
    "lift_sensor_up": false,
    "lift_sensor_down": false,
    "lift_homing": false,
    "dmc_enabled": false,
    "steering_enabled": false,
    "steering_left_volt": 0,
    "steering_right_volt": 0,
    "steering_output_volt": 0,
    "steering_manual": 0,
    "steering_control_percentage": 0,
    "steering_analog_out_enabled": false
}

// Error message
const msg_history = [];
function addMessage(msg) {
    msg_history.push(msg)
    displayMessage(msg);
}
function displayMessage(msg) {
    document.querySelector("#terminal").innerHTML = msg
}

// Set indicator
function setIndicator(indicator, value, color = "emerald") {
    const elm = document.querySelector(`#${indicator}.indicator`);
    elm.classList.add(value ? `bg-${color}-500` : "bg-slate-300")
    elm.classList.remove(value ? "bg-slate-300" : `bg-${color}-500`)
}

// Set Checkbox
function setChecked(checkbox, value) {
    const elm = document.querySelector(`input#${checkbox}`);
    elm.checked = value;
}

// Set number
function setValue(checkbox, value) {
    const elm = document.querySelector(`input#${checkbox}`);
    elm.value = value;
}

function refreshData(data) {
    // TODO: Fetch data
    setIndicator("emergency_stop", data['emergency_stop'], 'rose');
    setChecked("lift_enable", data["lift_enabled"]);

    setIndicator("retracted", data['lift_sensor_up'])
    setIndicator("extended", data['lift_sensor_down'])

    setIndicator("dmc_enabled", data['dmc_enabled'])

    setChecked("steering_enable", data["steering_enable"]);
    setIndicator("steering_enabled", data["steering_enabled"]);
    setChecked("output_enabled", data["steering_analog_out_enabled"]);

    setValue("offset_left", data["steering_left_volt"]);
    setValue("offset_right", data["steering_right_volt"]);
    setValue("steering_value", data["steering_control_percentage"]);
    setValue("steering_manual", data["steering_manual"]);
}


