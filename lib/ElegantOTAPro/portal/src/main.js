import './style.css'

import { uploadFile } from './upload';

// Validate files before upload
const validateFiles = (files) => {
    if (files.length > 1 && !multiple) {
        alert("You can only upload one (.bin) file at a time.");
        return false;
    }

    if (files[0].name.split('.').pop() != "bin") {
        alert("You can only upload (.bin) files.");
        return false;
    }

    return true;
}

// Dropzone Logic
var dropzone = document.getElementById('dropzone');
var dropzone_input = document.getElementById('fileInput');
var multiple = false;

['drag', 'dragstart', 'dragend', 'dragover', 'dragenter', 'dragleave', 'drop'].forEach(function(event) {
    dropzone.addEventListener(event, function(e) {
        e.preventDefault();
        e.stopPropagation();
    });
});

dropzone.addEventListener('dragover', function(e) {
    this.classList.add('dark:border-blue-500');
    this.classList.add('dark:border-opacity-80');
    this.classList.add('border-blue-500');
    this.classList.add('border-opacity-100');
}, false);

dropzone.addEventListener('dragleave', function(e) {
    this.classList.remove('dark:border-blue-500');
    this.classList.remove('dark:border-opacity-80');
    this.classList.remove('border-blue-500');
    this.classList.remove('border-opacity-100');
}, false);

dropzone.addEventListener('drop', function(e) {
    this.classList.remove('dark:border-blue-500');
    this.classList.remove('dark:border-opacity-80');
    this.classList.remove('border-blue-500');
    this.classList.remove('border-opacity-100');
    let files = e.dataTransfer.files;
    let dataTransfer = new DataTransfer();

    if (!validateFiles(files)) {
        return false;
    }

    // Start upload process
    uploadFile(files[0]);

    dropzone_input.files = files;
}, false);

dropzone.addEventListener('click', function(e) {
    dropzone_input.click();
});
  
// Handle file selection
function onFileInput(files) {
    if (!validateFiles(files)) {
        return false;
    }
    // Start upload process
    uploadFile(files[0]);
    return;
}

// Reset View ( by refreshing the page )
function resetView() {
    // Refresh the page
    window.location.reload();
}

(async () => {
    // Load metadata
    const response = await fetch('/ota/metadata');
    if (response.status === 200) {
        const metadata = await response.json();
        // Parameters for testing
        // let metadata = {
        //     tt: 'ElegantOTA Title',
        //     lw: 220,
        //     lh: 68,
        //     fwm: true,
        //     fsm: true,
        //     hid: '12345678',
        //     fwv: '1.0.0',
        // }

        // Set title
        document.title = metadata.tt;

        // Disable firmware option if not supported
        if (metadata.fwm === false) {
            document.getElementById('fwm').remove()
        }

        if (metadata.fsm === false) {
            document.getElementById('fsm').remove()
        }

        if (metadata.fwm === false && metadata.fsm === false) {
            document.getElementById("modes").options[0].selected = true;
            document.getElementById("disabledColumn").classList.remove('hidden');
            document.getElementById("uploadColumn").classList.add('hidden');
        } else {
            document.getElementById("modes").options[1].selected = true;
        }

        // Make upload column visible
        document.getElementById("uploadColumn").classList.remove('select-none');
        document.getElementById("uploadColumn").classList.remove('pointer-events-none');
        document.getElementById("uploadColumn").classList.remove('opacity-40');

        // Unhide OTA modes
        document.getElementById('modes').classList.remove('hidden');

        // Hide modes spinner
        document.getElementById('modesSpinner').classList.add('hidden');

        // Set hardware id
        document.getElementById('hid').innerHTML = metadata.hid;
        // Hide hardware id spinner
        document.getElementById('hidSpinner').classList.add('hidden');

        // Set firmware version
        document.getElementById('fwv').innerHTML = metadata.fwv;
        // Hide firmware version spinner
        document.getElementById('fwvSpinner').classList.add('hidden');

        // Load logos
        setTimeout(() => {
            if (document.getElementById("darkModeCheckbox").checked == true) {
                document.getElementById("logo").src = "/ota/logo/dark";
            } else {
                document.getElementById("logo").src = "/ota/logo/light";
            }
            // Set width and height of logo
            document.getElementById('logo').width = metadata.lw;
            document.getElementById('logo').height = metadata.lh;
        }, 200);
    } else {
        console.error('Failed to get metadata!')
    }
})();

window.onFileInput = onFileInput;
window.resetView = resetView;