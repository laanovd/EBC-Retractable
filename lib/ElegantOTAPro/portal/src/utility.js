
export const showElement = (element) => {
    document.getElementById(element).classList.remove('hidden');
}

export const hideElement = (element) => {
    document.getElementById(element).classList.add('hidden');
}

export const setProgressTitle = (title) => {
    document.getElementById("progressTitle").innerHTML = title;
}

export const setErrorTitle = (title) => {
    document.getElementById("errorTitle").innerHTML = title;
}

export const setErrorReason = (reason) => {
    document.getElementById("errorReason").innerHTML = reason;
}