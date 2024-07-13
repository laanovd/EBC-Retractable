// Emergency stop
const elm_id_maintenance_on = "maintenance_button_on"
const elm_id_maintenance_off = "maintenance_button_off"
const elm_id_maintenance_loading = "maintenance_button_loading"
const json_key_maintenance = "maintenance_enable"

function INIT_maintenance_button() {
    const btn_on = document.querySelector(`#${elm_id_maintenance_on}`);
    const btn_off = document.querySelector(`#${elm_id_maintenance_off}`);
    btn_on.addEventListener("click", () => {
        if (btn_on.disabled) return;
        sendCommand(JSON.stringify({ [json_key_maintenance]: false }));
        btn_on.disabled = true;
    })
    btn_off.addEventListener("click", () => {
        if (btn_off.disabled) return;
        sendCommand(JSON.stringify({ [json_key_maintenance]: true }));
        btn_off.disabled = true;
    })
    addMessageHandler((data) => {
        set_maintenance_button(data[json_key_maintenance]);
    })
}
function set_maintenance_button(value) {
    const btn_on = document.querySelector(`#${elm_id_maintenance_on}`);
    const btn_off = document.querySelector(`#${elm_id_maintenance_off}`);
    const btn_loading = document.querySelector(`#${elm_id_maintenance_off}`);
    (value ? btn_on : btn_off).classList.remove("hidden");
    (value ? btn_off : btn_on).classList.add("hidden");
    btn_on.disabled = false;
    btn_off.disabled = false;
}

// Emergency stop
const elm_id_emergency_stop = "emergency_stop_indicator"
const json_key_emergency_stop = "emergency_stop"
function INIT_emergency_stop() {
    addMessageHandler((data) => {
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
function INIT_extend_enable() {
    const elm = document.querySelector(`#${elm_id_extend}`);
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_extend]: e.target.checked }));
    });
    addMessageHandler((data) => {
        set_extending(data[json_key_extend]);
    })
}
function set_extending(value) {
    document.querySelector(`input#${elm_id_extend}`).checked = value;
}

// Retracted
const elm_id_retracted = "extended_indicator"
const json_key_retracted = "lift_sensor_up"
function INIT_retracted_indicator() {
    addMessageHandler((data) => {
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
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_retract]: e.target.checked }));
    });
    addMessageHandler((data) => {
        set_retracting(data[json_key_retract]);
    })
}
function set_retracting(value) {
    document.querySelector(`input#${elm_id_retract}`).checked = value;
}

// dmc enable
const elm_id_dmc_enable_indicator = "dmc_enable_indicator"
const elm_id_dmc_enable = "dmc_enable"
const json_key_dmc_enable = "dmc_enable"
function INIT_dmc_enable() {
    const elm = document.querySelector(`#${elm_id_dmc_enable}`);
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_dmc_enable]: e.target.checked }));
        set_dmc_enabled(e.target.checked)
    });
    addMessageHandler((data) => {
        set_dmc_enabled(data[json_key_dmc_enable]);
    })
}
function set_dmc_enabled(value) {
    setChecked(elm_id_dmc_enable, value)
    const elm = document.querySelector(`#${elm_id_dmc_enable_indicator}`);
    elm.classList.add(value ? `bg-emerald-500` : "bg-slate-300")
    elm.classList.remove(value ? "bg-slate-300" : `bg-emerald-500`)
}

// Lift Enable
const elm_id_lift_enable = "lift_enable"
const json_key_lift_enable = "lift_enabled"
function INIT_lift_enable() {
    const elm = document.querySelector(`#${elm_id_lift_enable}`);
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_lift_enable]: e.target.checked }));
    });
    addSingleMessageHandler
}
function lift_enable_set_value(value) {
    setValue(elm_id_lift_enable, value)
}

// Steering Enable
const elm_id_steering_enable = "steering_enable"
const elm_id_steering_enabled = "steering_enabled"
const json_key_steering_enable = "steering_enabled"
function INIT_steering_enable() {
    const elm = document.querySelector(`#${elm_id_steering_enable}`);
    console.log(elm)
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_steering_enable]: e.target.checked }));
    });
    addMessageHandler((data) => {
        steering_enable_set_value(data[json_key_steering_enable]);
    })
}
function steering_enable_set_value(value) {
    setChecked(elm_id_steering_enable, value)
    const elm = document.querySelector(`#${elm_id_steering_enabled}`);
    elm.classList.add(value ? `bg-emerald-500` : "bg-slate-300")
    elm.classList.remove(value ? "bg-slate-300" : `bg-emerald-500`)
}

// Output Enable
const elm_id_output_enable = "output_enabled"
const json_key_output_enable = "steering_analog_out_enabled"
function INIT_output_enable() {
    const elm = document.querySelector(`#${elm_id_output_enable}`);
    console.log(elm)
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_output_enable]: e.target.checked }));
    });
    addMessageHandler((data) => {
        output_enable_set_value(data[json_key_output_enable]);
    })
}
function output_enable_set_value(value) {
    setChecked(elm_id_output_enable, value)
}

// Steering offset right
const elm_id_offset_right = "offset_right"
const json_key_ofsset_right = "steering_right_volt"
function INIT_steering_offset_right() {
    const elm = document.querySelector(`#${elm_id_offset_right}`);
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_ofsset_right]: e.target.value }));
    });
    addSingleMessageHandler((data) => {
        steering_offset_right_set_value(data[json_key_ofsset_right]);
    })
}
function steering_offset_right_set_value(value) {
    setValue(elm_id_offset_right, value)
}

// Steering offset left
const elm_id_offset_left = "offset_left"
const json_key_offset_left = "steering_left_volt"
function INIT_steering_offset_left() {
    const elm = document.querySelector(`#${elm_id_offset_left}`);
    elm.addEventListener("change", (e) => {
        sendCommand(JSON.stringify({ [json_key_offset_left]: e.target.value }));
    });
    addSingleMessageHandler((data) => {
        steering_offset_left_set_value(data[json_key_offset_left]);
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
    INIT_extend_enable();
    INIT_retract_enable();
    INIT_lift_enable();
    INIT_steering_offset_right();
    INIT_steering_offset_left();
    INIT_steering_actual();
    INIT_steering_enable();
    INIT_steering();
    INIT_output_enable();
    INIT_dmc_enable();

    addSingleMessageHandler((data) => { document.querySelector("#loading_overlay").classList.add("hidden") });
}, false);