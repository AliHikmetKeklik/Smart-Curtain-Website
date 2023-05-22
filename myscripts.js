function showCurtain(curtainId) {
    var curtain1 = document.getElementById('curtain1');
    var curtain2 = document.getElementById('curtain2');

    if (curtainId === 'curtain1') {
        curtain1.removeAttribute('hidden');
        curtain2.setAttribute('hidden', 'true');
        secondCurtain.style.backgroundColor = 'gray';
        firstCurtain.style.backgroundColor = 'blue';
    } else if (curtainId === 'curtain2') {
        curtain1.setAttribute('hidden', 'true');
        curtain2.removeAttribute('hidden');
        firstCurtain.style.backgroundColor = 'gray';
        secondCurtain.style.backgroundColor = 'blue';
    }
}
//   CURTAIN 1


function sendToggleValue() {
    var toggleValue = document.getElementById("tempToggleButtonCurtain1").checked;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/handleToggleValueCurtain1?value=" + toggleValue, true);
    xhttp.send();
}

// Function to toggle the temperature switch when the sun switch is clicked
function toggleTemperature1() {
    var sun1Switch = document.querySelector('#sun1 input[type="checkbox"]');
    var temperature1Switch = document.querySelector('#temperature1 input[type="checkbox"]');

    if (sun1Switch.checked) {
        temperature1Switch.checked = false;
    }
}

// Add event listener to the sun switch
var sun1Switch = document.querySelector('#sun1 input[type="checkbox"]');
sun1Switch.addEventListener('click', toggleTemperature1);

// Function to toggle the sun switch when the temperature switch is clicked
function toggleSun1() {
    var sun1Switch = document.querySelector('#sun1 input[type="checkbox"]');
    var temperature1Switch = document.querySelector('#temperature1 input[type="checkbox"]');
    if (temperature1Switch.checked) {
        sun1Switch.checked = false;
    }
}

// Add event listener to the temperature switch
var temperature1Switch = document.querySelector('#temperature1 input[type="checkbox"]');
temperature1Switch.addEventListener('click', toggleSun1);


// Function to turn off temperature and sun toggles when the save button is clicked
function turnOffToggles1() {
    var temperature1Switch = document.querySelector('#temperature1 input[type="checkbox"]');
    var sun1Switch = document.querySelector('#sun1 input[type="checkbox"]');

    temperature1Switch.checked = false;
    sun1Switch.checked = false;
}

// Add event listener to the save button
var saveTimeBtn1 = document.querySelector('#saveTimeBtn1');
saveTimeBtn1.addEventListener('click', turnOffToggles1);

// CURTAIN 2

// Function to toggle the temperature switch when the sun switch is clicked
function toggleTemperature2() {
    var sun2Switch = document.querySelector('#sun2 input[type="checkbox"]');
    var temperature2Switch = document.querySelector('#temperature2 input[type="checkbox"]');

    if (sun2Switch.checked) {
        temperature2Switch.checked = false;
    }
}

// Add event listener to the sun switch
var sun2Switch = document.querySelector('#sun2 input[type="checkbox"]');
sun2Switch.addEventListener('click', toggleTemperature2);

// Function to toggle the sun switch when the temperature switch is clicked
function toggleSun2() {
    var sun2Switch = document.querySelector('#sun2 input[type="checkbox"]');
    var temperature2Switch = document.querySelector('#temperature2 input[type="checkbox"]');
    if (temperature2Switch.checked) {
        sun2Switch.checked = false;
    }
}

// Add event listener to the temperature switch
var temperature2Switch = document.querySelector('#temperature2 input[type="checkbox"]');
temperature2Switch.addEventListener('click', toggleSun2);


// Function to turn off temperature and sun toggles when the save button is clicked
function turnOffToggles2() {
    var temperature2Switch = document.querySelector('#temperature2 input[type="checkbox"]');
    var sun2Switch = document.querySelector('#sun2 input[type="checkbox"]');

    temperature2Switch.checked = false;
    sun2Switch.checked = false;
}

// Add event listener to the save button
var saveTimeBtn2 = document.querySelector('#saveTimeBtn2');
saveTimeBtn2.addEventListener('click', turnOffToggles2);