function showTransfer(visible) {
    let array = document.getElementsByClassName("transfer-indicator")
    for (let index = 0; index < array.length; index++) {
        array[index].style.display = visible ? "block" : "none";
    }
}

function showError(text) {
    document.getElementById('messagebox').hidden = false;
    document.getElementById('message').textContent = text;
}

function hideError() {
    document.getElementById('messagebox').hidden = true;
}

hideError();
showTransfer(false);

function updateValue(url, elementName) {
    const request = new XMLHttpRequest();
    request.open('GET', url);
    request.send();
    request.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (this.responseText != '') {
                document.getElementById(elementName).innerHTML = this.responseText;
            }
            else
            {
                showError('Leere Antwort für "' + url + '" erhalten!');
            }
        } else if (this.readyState == 4 && this.status != 200) {
            showError('Werte konnten nicht abgefragt werden! (Statuscode: ' + this.status + ')')
        }
    };
}

function update() {
    hideError();
    showTransfer(true);
    updateValue('/temp', 'temperature');
    updateValue('/hum', 'humidity');
    showTransfer(false);
}

setInterval(update, 2000);
update();