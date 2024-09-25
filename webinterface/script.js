let device;
let server;
let service;
let characteristic;

const connectButton = document.getElementById('connectButton');
const speedControl = document.getElementById('speed');
const controls = document.getElementById('controls');
const settingsButton = document.getElementById('settingsButton');
const settingsModal = document.getElementById('settingsModal');
const closeModal = document.querySelector('.close');
const saveSettingsButton = document.getElementById('saveSettings');
const accelerationControl = document.getElementById('acceleration');
const brakePowerControl = document.getElementById('brakePower');
const dircheckbox = document.getElementById('dir');
const statusspan = document.getElementById('status');
const battery = document.getElementById('battery');
const light = document.getElementById('light');


// UUIDs for the train BLE service and characteristic (you'll need to adjust these)
const SERVICE_UUID = "2669f374-8734-4f2e-8045-9e52781f078c";
const CHARACTERISTIC_UUID = "8466ab14-a8d5-4231-b0a6-630e59501bb8";
let speedValue = 0;

// Function to connect to the Bluetooth device
async function connect() {
    connectButton.disabled=true;
    try {
        device = await navigator.bluetooth.requestDevice({
            filters: [{ services: [SERVICE_UUID] }]
        });
        server = await device.gatt.connect();
        service = await server.getPrimaryService(SERVICE_UUID);
        characteristic = await service.getCharacteristic(CHARACTERISTIC_UUID);
        //await characteristic.startNotifications();

        
        // Display controls once connected
        controls.style.display = 'block';
        let lastSpeed = 0;
        speedControl.value = 0;
        dircheckbox.checked = false;
        let lastsend = 0;
        setInterval(()=>{
            let now = performance.now()/1000;
            if(lastSpeed!=speedValue){
                lastSpeed=speedValue;
                sendSpeed(speedValue);
                lastsend = now;
            }else{
                if((lastsend+0.5)<now){
                    //sendSpeed(speedValue);
                    lastsend=now;
                }
            }
        },100);
        
        statusspan.innerText=('connected')

        characteristic.startNotifications();
        characteristic.addEventListener('characteristicvaluechanged',e=>{
            console.log(e.target.value);
            const decoder = new TextDecoder();
            const str = decoder.decode(e.target.value);
            battery.textContent = ""+str;
        });
        
    } catch (error) {
        console.error('Failed to connect:', error);
        statusspan.innerText=('Failed to connect:'+String(error))
        connectButton.disabled=false;

    }
}
function ab2str(buf) {
    return new TextDecoder().decode(uint8array);
}
function str2ab(txt) {
    var uint8array = new TextEncoder("utf-8").encode(txt);
    return uint8array;
}
let isSending = false;
async function sendCmd(cmd,val) {
    if (characteristic) {
        const text = cmd + val;
        const ab = str2ab(text)
        if(isSending)return;
        isSending = true;
        const res = await characteristic.writeValue(ab);
        isSending = false;
        console.log(res, text, ab);
    }
}
async function sendSpeed(speed) {
    if (characteristic) {
        await sendCmd("s",speed)
    }
}

// Function to stop the train
async function stop() {
    speedValue = 0;
    speedControl.value = 0;
    await sendSpeed("s",0);  // Send 0 to stop the train
}
async function senddir() {
    await sendCmd("d",Number(dircheckbox.checked));
}
async function playsound(index) {
    await sendCmd("p",index);
}
// Event listeners
connectButton.addEventListener('click', connect);
speedControl.addEventListener('input', () => (speedValue=speedControl.value));
dircheckbox.addEventListener("input",senddir)

// Open settings modal
settingsButton.addEventListener('click', () => {
    settingsModal.style.display = 'flex';
});

// Close modal when 'x' is clicked
closeModal.addEventListener('click', () => {
    settingsModal.style.display = 'none';
});
light.onclick=()=>{
    sendCmd("l",Number(light.checked));
}

// Save settings and send to train
saveSettingsButton.addEventListener('click', () => {
    const acceleration = parseInt(accelerationControl.value);
    const brakePower = parseInt(brakePowerControl.value);

    // Send the acceleration and brake power to the train
    sendSettings(acceleration, brakePower);

    // Close the modal after saving
    settingsModal.style.display = 'none';
});

// Close modal if user clicks outside the modal content
window.addEventListener('click', (event) => {
    if (event.target == settingsModal) {
        settingsModal.style.display = 'none';
    }
});
