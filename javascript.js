// ************************************************
window.addEventListener('DOMContentLoaded', init, false);

function init() {
    fetch('firmware/versions.json')
    .then(response => response.json())
    .then(data => {
        const versions = data;
        GenerateSelectList(versions);
        checkSupported(); 
        resetCheckboxes();
    })
    .catch(error => console.error('Error loading versions:', error));
}

function showSerialHelp() {
    document.getElementById('coms').innerHTML = `Hit "Install" and select the correct COM port.<br><br>
    You might be missing the drivers for your board.<br>
    Here are drivers for chips commonly used in ESP boards:<br>
    <a href="https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers" target="_blank">CP2102 (square chip)</a><br>
    <a href="https://github.com/nodemcu/nodemcu-devkit/tree/master/Drivers" target="_blank">CH34x (rectangular chip)</a><br><br>
    Make sure your USB cable supports data transfer.<br><br>
    `;
}

function checkSupported() {
    if (document.getElementById('web-install-button').hasAttribute('install-unsupported')) unsupported();
    else setManifest();
}

function unsupported() {
    document.getElementById('flasher').innerHTML = `Sorry, your browser is not yet supported!<br>
    Please try on Desktop Chrome or Edge.<br>
    Find binary files here:<br>
    <a href="https://github.com/tobiasfaust/WLED/releases" target="_blank">
    <button class="btn" slot="activate">GitHub Releases</button>
    </a>`
}

function resetCheckboxes() {
    const radioButtons = document.querySelectorAll('input[type="radio"]');
    radioButtons.forEach(radio => {
        radio.checked = false;
        radio.disabled = false;
    });
}

// json: {path: 'firmware/v2.5.0-237-development/manifest_all.json', version: 'v2.5.0-237', stage: 'development', build: 237}
function GenerateSelectList(json) {
    const select = document.getElementById('versions');
    const stages = {};

    // Group by stage
    json.forEach(obj => {
        if (!stages[obj.stage]) {
            stages[obj.stage] = [];
        }
        stages[obj.stage].push(obj);
    });

    // Sort each stage by build number in descending order
    for (const stage in stages) {
        stages[stage].sort((a, b) => b.build - a.build);
    }

    // Create optgroups and options
    for (const stage in stages) {
        const optgroup = document.createElement('optgroup');
        optgroup.label = stage;
        stages[stage].forEach(obj => {
            const option = document.createElement('option');
            option.value = obj.path;
            option.text = obj.version + " (Build " + obj.build + ")";
            optgroup.appendChild(option);
        });
        select.appendChild(optgroup);
    }
}

function setManifest() {
    var sel = document.getElementById('versions');
    var opt = sel.options[sel.selectedIndex];
    var m = opt.value;
    document.getElementById('web-install-button').setAttribute('manifest', m);
}


