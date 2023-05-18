function showCurtain(curtainId) {
    var curtain1 = document.getElementById('curtain1');
    var curtain2 = document.getElementById('curtain2');

    if (curtainId === 'curtain1') {
        curtain1.removeAttribute('hidden');
        curtain2.setAttribute('hidden', 'true');
    } else if (curtainId === 'curtain2') {
        curtain1.setAttribute('hidden', 'true');
        curtain2.removeAttribute('hidden');
    }
}
//   CURTAIN 1
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


// FIREBASE

/*// Import the functions you need from the SDKs you need
        import { initializeApp } from "firebase/app";
        import { getAnalytics } from "firebase/analytics";
        // TODO: Add SDKs for Firebase products that you want to use
        // https://firebase.google.com/docs/web/setup#available-libraries
    
        // Your web app's Firebase configuration
        // For Firebase JS SDK v7.20.0 and later, measurementId is optional
        const firebaseConfig = {
          apiKey: "AIzaSyDqWPZkV-JshuEaqQjaKLbgXm75tX6smo8",
          authDomain: "webbtry.firebaseapp.com",
          databaseURL: "https://webbtry-default-rtdb.europe-west1.firebasedatabase.app",
          projectId: "webbtry",
          storageBucket: "webbtry.appspot.com",
          messagingSenderId: "619267636692",
          appId: "1:619267636692:web:578f080f1598858f4d2a41",
          measurementId: "G-5D3039H5E4"
        };
    
        // Initialize Firebase
        const app = initializeApp(firebaseConfig);
        //const analytics = getAnalytics(app);
    
        import { getDatabase, set, get, update, remove, ref, childe }
          from "firebase/database";
    
    
        const db = getDatabase();*/