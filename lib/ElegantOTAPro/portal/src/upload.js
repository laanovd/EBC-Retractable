import md5 from 'md5'

import {
    hideElement,
    setErrorReason,
    setErrorTitle,
    setProgressTitle,
    showElement
} from "./utility";

const getFileHash = async (file) => {
    return new Promise((resolve, reject) => {
        let hash = '';
        let reader = new FileReader();
        reader.onload = function(e) {
            hash = md5(e.target.result);
            resolve(hash);
        };
        reader.readAsArrayBuffer(file);
    });
}

export const uploadFile = async (file) => {
    // First, hide the dropzone
    hideElement("uploadColumn");
    hideElement("settingsColumn");
    showElement("progressColumn");

    // Get OTA mode from settings
    let otaMode = document.getElementById("modes").value;

    if (otaMode === "") {
        // Upload failed
        hideElement("progressColumn");
        showElement("errorColumn");
        setErrorTitle("Upload failed");
        setErrorReason("No OTA mode selected");
        return;
    }

    try {
        // Generate MD5 hash of file
        let hash = await getFileHash(file);

        // Then, start OTA process
        setProgressTitle("Starting OTA Process");
        
        // Start OTA process with query string
        const request = await fetch(`/ota/start?mode=${otaMode}&hash=${hash}`);

        if (!request.ok) {
            throw new Error('Start OTA process failed');
        }

        const response = await request.text();
        console.log('Start OTA response:', response);

        // Create XHR request to /eota/upload
        const formData = new FormData();
        let xhr = new XMLHttpRequest();

        // Send file
        xhr.open("POST", "/ota/upload");

        // Update progress bar
        xhr.upload.addEventListener("progress", function(e) {
            let percent = Math.round((e.loaded / e.total) * 100);
            document.getElementById("progressBar").style.width = percent + "%";
            document.getElementById("progressValue").innerHTML = percent + "%";
        }, false);

        xhr.upload.onprogress = function(e) {
            if (e.lengthComputable) {
                let percent = Math.round((e.loaded / e.total) * 100);
                document.getElementById("progressBar").style.width = percent + "%";
                document.getElementById("progressValue").innerHTML = percent + "%";
            }
        };

        // Handle upload completion
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4) {
                if (xhr.status == 200) {
                    // Upload complete
                    document.getElementById("progressBar").style.width = "100%";
                    document.getElementById("progressBar").innerHTML = "100%";
                    hideElement("progressColumn");
                    showElement("successColumn");
                } else if (xhr.status == 400) {
                    // Upload failed
                    document.getElementById("progressBar").style.width = "100%";
                    document.getElementById("progressBar").innerHTML = "100%";
                    hideElement("progressColumn");
                    showElement("errorColumn");
                    setErrorTitle("Upload failed");
                    // Get reason from json response
                    let response = xhr.responseText;
                    setErrorReason(response);
                } else {
                    // Upload failed
                    document.getElementById("progressBar").style.width = "100%";
                    document.getElementById("progressBar").innerHTML = "100%";
                    hideElement("progressColumn");
                    showElement("errorColumn");
                    setErrorTitle("Upload failed");
                    setErrorReason("Server returned status code " + xhr.status);
                }
            }   
        }
        // Append file to form data
        formData.append("file", file, file.name);
        xhr.send(formData);

        // Then, show the upload progress bar with file name
        setProgressTitle("Uploading " + file.name);
    } catch (err) {
        // Upload failed
        hideElement("progressColumn");
        showElement("errorColumn");
        setErrorTitle("Upload failed");
        setErrorReason(err.message);
    }
}