// Emergency stop
const elm_id_maintenance_enabled = "maintenance_enabled"
const elm_id_maintenance_on = "maintenance_button_on"
const elm_id_maintenance_off = "maintenance_button_off"
const elm_id_maintenance_loading = "maintenance_button_loading"
const json_key_maintenance_enabled = "maintenance_enabled"

function INIT_maintenance_button() {
    const elm = document.querySelector(`#${elm_id_maintenance_enabled}`)
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_maintenance_enabled]: e.target.checked }));
    })

    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_maintenance_enabled)) {
            set_maintenance_button(JSON.parse(data[json_key_maintenance_enabled]));
        }
    })
}
function set_maintenance_button(value) {
    const elm = document.querySelector(`#${elm_id_maintenance_enabled}`)
    elm.checked = value;
    elm.disabled = false;
}

// Emergency stop
const elm_id_emergency_stop = "emergency_stop_indicator"
const json_key_emergency_stop = "emergency_stop"
function INIT_emergency_stop() {
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_emergency_stop))
            set_emergency_stop(data[json_key_emergency_stop]);
    })
}
function set_emergency_stop(value) {
    const elm = document.querySelector(`#${elm_id_emergency_stop}`);
    elm.classList.add(value ? `bg-rose-500` : "bg-slate-300")
    elm.classList.remove(value ? "bg-slate-300" : `bg-rose-500`)
}

// Extended
const elm_id_extended = "extended_indicator"
const json_key_extended = "lift_sensor_down"
function INIT_extended_indicator() {
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_extended))
            set_extended_indicator(data[json_key_extended]);
    })
}
function set_extended_indicator(value) {
    const elm = document.querySelector(`#${elm_id_extended}`);
    elm.classList.add(value ? `bg-rose-500` : "bg-slate-300")
    elm.classList.remove(value ? "bg-slate-300" : `bg-rose-500`)
}
const elm_id_extend = "extend"
const json_key_extend = "lift_motor_down"
function INIT_extend_enabled() {
    const elm = document.querySelector(`#${elm_id_extend}`);
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_extend]: e.target.checked }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_extend))
            set_extending(JSON.parse(data[json_key_extend]));
    })
}
function set_extending(value) {
    const elm = document.querySelector(`input#${elm_id_extend}`);
    elm.checked = value;
    elm.disabled = false;
}

// Retracted
const elm_id_retracted = "extended_indicator"
const json_key_retracted = "lift_sensor_up"
function INIT_retracted_indicator() {
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_retract))
            set_retracted_indicator(data[json_key_retract]);
    })
}
function set_retracted_indicator(value) {
    const elm = document.querySelector(`#${elm_id_extended}`);
    elm.classList.add(value ? `bg-rose-500` : "bg-slate-300")
    elm.classList.remove(value ? "bg-slate-300" : `bg-rose-500`)
}
const elm_id_retract = "retract"
const json_key_retract = "lift_motor_up"
function INIT_retract_enable() {
    const elm = document.querySelector(`#${elm_id_retract}`);
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_retract]: e.target.checked }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_retract))
            set_retracting(JSON.parse(data[json_key_retract]));
    })
}
function set_retracting(value) {
    const elm = document.querySelector(`input#${elm_id_retract}`)
    elm.checked = value;
    elm.disabled = false;
}

// dmc enable
const elm_id_dmc_enabled = "dmc_enabled"
const json_key_dmc_enabled = "dmc_enabled"
function INIT_dmc_enabled() {
    const elm = document.querySelector(`#${elm_id_dmc_enabled}`);
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_dmc_enabled]: e.target.checked }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_dmc_enabled))
            set_dmc_enabled(JSON.parse(data[json_key_dmc_enabled]));
    })
}
function set_dmc_enabled(value) {
    const elm = document.querySelector(`#${elm_id_dmc_enabled}`);
    elm.checked = value;
    elm.disabled = false;
}

// Lift Enable
const elm_id_lift_enabled = "lift_enabled"
const json_key_lift_enabled = "lift_enabled"
function INIT_lift_enable() {
    const elm = document.querySelector(`#${elm_id_lift_enabled}`);
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_lift_enabled]: e.target.checked }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_lift_enabled))
            lift_enable_set_value(JSON.parse(data[json_key_lift_enabled]));
    })
}
function lift_enable_set_value(value) {
    const elm = document.querySelector(`#${elm_id_lift_enabled}`);
    elm.checked = value;
    elm.disabled = false;
}

// Steering Enable
const elm_id_steering_enabled = "steering_enabled"
const json_key_steering_enabled = "steering_enabled"
function INIT_steering_enabled() {
    const elm = document.querySelector(`#${elm_id_steering_enabled}`);
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_steering_enabled]: e.target.checked }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_steering_enabled))
            steering_enabled_set_value(JSON.parse(data[json_key_steering_enabled]));
    })
}
function steering_enabled_set_value(value) {
    const elm = document.querySelector(`#${elm_id_steering_enabled}`)
    elm.checked = value;
    elm.disabled = false;
}

// Output Enable
const elm_id_output_enabled = "output_enabled"
const json_key_output_enabled = "steering_analog_out_enabled"
function INIT_output_enabled() {
    const elm = document.querySelector(`#${elm_id_output_enabled}`);
    elm.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        elm.disabled = true;
        sendCommand(JSON.stringify({ [json_key_output_enabled]: e.target.checked }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_output_enabled))
            output_enabled_set_value(JSON.parse(data[json_key_output_enabled]));
    })
}
function output_enabled_set_value(value) {
    const elm = document.querySelector(`#${elm_id_output_enabled}`);
    elm.checked = value;
    elm.disabled = false;
}

// Steering offset right
const elm_id_offset_right = "offset_right"
const json_key_offset_right = "steering_right_volt"
let offset_right_current_value = 0;
function INIT_steering_offset_right() {
    const elm = document.querySelector(`#${elm_id_offset_right}`);
    elm.addEventListener("change", (e) => {
        elm.value = offset_right_current_value;
        const val = e.target.value;
        if (val >= 0 && val <= 5 && val < offset_left_current_value)
            sendCommand(JSON.stringify({ [json_key_offset_right]: e.target.value }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_offset_right)) {
            const val = data[json_key_offset_right];
            offset_right_current_value = val;
            steering_offset_right_set_value(val);
        }
    })
}
function steering_offset_right_set_value(value) {
    setValue(elm_id_offset_right, value)
}

// Steering offset left
const elm_id_offset_left = "offset_left"
const json_key_offset_left = "steering_left_volt"
let offset_left_current_value = 0;
function INIT_steering_offset_left() {
    const elm = document.querySelector(`#${elm_id_offset_left}`);
    elm.addEventListener("change", (e) => {
        elm.value = offset_left_current_value;
        const val = e.target.value;
        if (val >= 0 && val <= 5 && val < offset_right_current_value)
            sendCommand(JSON.stringify({ [json_key_offset_left]: e.target.value }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_offset_left)) {
            const val = data[json_key_offset_left];
            offset_val_current_value = val;
            steering_offset_left_set_value(val);
        }
    })
}
function steering_offset_left_set_value(value) {
    setValue(elm_id_offset_left, value)
}

// Steering actual
const elm_id_steering_actual = "steering_manual"
const json_key_steering_actual = "steering_output_volt"
function INIT_steering_actual() {
    const elm = document.querySelector(`#${elm_id_steering_actual}`);
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_steering_actual))
            steering_manual_set_value(data[json_key_steering_actual]);
    })
}
function steering_manual_set_value(value) {
    setValue(elm_id_steering_actual, value)
}

// Steering
const elm_id_steering = "steering_value"
const json_key_steering = "steering_manual"
function INIT_steering() {
    const elm = document.querySelector(`#${elm_id_steering}`);
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_steering]: e.target.value }));
    });
    addMessageHandler((data) => {
        if (data.hasOwnProperty(json_key_steering))
            steering_set_value(data[json_key_steering]);
    })
}
function steering_set_value(value) {
    setValue(elm_id_steering, value)
}


// Init
document.addEventListener('DOMContentLoaded', function () {
    INIT_maintenance_button();
    INIT_emergency_stop();
    INIT_extended_indicator();
    INIT_retracted_indicator();
    INIT_extend_enabled();
    INIT_retract_enable();
    INIT_lift_enable();
    INIT_steering_offset_right();
    INIT_steering_offset_left();
    INIT_steering_actual();
    INIT_steering_enabled();
    INIT_steering();
    INIT_output_enabled();
    INIT_dmc_enabled();

    addSingleMessageHandler((data) => { document.querySelector("#loading_overlay").classList.add("hidden") });
}, false);