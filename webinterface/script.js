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
const realspeed = document.getElementById('realspeed');
const soundvol = document.getElementById('soundvol');
const motorsound = document.getElementById('motorsound');

const batterieStatus = document.getElementById("batterie-status");
const ladenStatus = document.getElementById("laden-status");
const vollStatus = document.getElementById("voll-status");
const reedStatus = document.getElementById("reed-status");
const pendelbetriebStatus = document.getElementById("pendelbetrieb-status");
const richtungStatus = document.getElementById("richtung-status");

// UUIDs for the train BLE service and characteristic (you'll need to adjust these)
const SERVICE_UUID = "2669f374-8734-4f2e-8045-9e52781f078c";
const CHARACTERISTIC_UUID = "8466ab14-a8d5-4231-b0a6-630e59501bb8";
let speedValue = 0;
let speedtimeoutinterval = setInterval(e=>{},1000);
let halt = false;

reed = document.getElementById("reed");
settingslist = [
    revertlock,
    enhyst,
    hysteresis,
    soundvol,
    motorsound,
    soundmotor,
    reed
];
for (let i = 0; i < settingslist.length; i++) {
    const ch = settingslist[i];
    if(ch.type=="checkbox"){
        ch.addEventListener("change",e=>{
            const id = i;
            sendCmd("e",id+" "+Number(ch.checked));
        });
    }
}
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
        setInterval(async ()=>{
            let now = performance.now()/1000;
            if(halt){
                if(await sendCmd("h",0)){
                    halt = false;
                    return;
                }
            }
            if(lastSpeed!=speedValue){
                lastSpeed=speedValue;
                await sendSpeed(speedValue);
                lastsend = now;
            }else{
                if((lastsend+0.5)<now){
                    await sendSpeed(speedValue);
                    lastsend=now;
                }
            }
        },100);
        
        statusspan.innerText=('connected')
        characteristic.startNotifications();
        characteristic.addEventListener('characteristicvaluechanged',e=>{
            //console.log(e.target.value);
            const decoder = new TextDecoder();
            const lines = decoder.decode(e.target.value);
            //console.log(str)
            lines.split("\n").forEach(str=>{
                const cmd = str.charAt(0);
                const val = str.substring(1);
                if(cmd=='b'){
                    battery.textContent = ""+val;
                }
                if(cmd=='v'){
                    realspeed.value = Number(val);
                }
                if(cmd=='d'){
                    dircheckbox.checked = Number(val)!=0;
                }
                if(cmd=='r'){
                    reedStatus.style.background=Number(val)!=0?"red":"gray";
                }
                if(cmd=='a'){
                    splt = val.split(" ").splice(1);
                    console.log(splt)
                    for (let i = 0; i < settingslist.length; i++) {
                        const e = settingslist[i];
                        if(e.type=="range"){
                            e.value = Number(splt[i]);
                        }
                        if(e.type=="checkbox"){
                            e.checked = Number(splt[i])!=0;
                        }
                    }
                    update();
                }
            });
        });
        
    } catch (error) {
        console.error('Failed to connect:', error);
        statusspan.innerText=('Failed to connect:'+String(error))
        connectButton.disabled=false;

    }
}
function ab2str(buf) {
    return new TextDecoder().decode(buf);
}
function str2ab(txt) {
    var uint8array = new TextEncoder("utf-8").encode(txt);
    return uint8array;
}
let isSending = false;
let sendqueue = [];
async function sendCmd(cmd,val) {
    if (characteristic) {
        const text = cmd + val;
        let ab = str2ab(text)
        sendqueue.push(ab);

        if(isSending)return true;
        isSending = true;
        //let res = await characteristic.writeValue(ab);
        //console.log(res, text, ab);
        //console.log(ab2str(ab));
        while(sendqueue.length>0){
            ab = sendqueue.shift();
            
            res = await characteristic.writeValue(ab);
            //console.log(res, ab);
            console.log(ab2str(ab));
        }
        isSending = false;
        return true;
    }
    return false;
}
async function sendSpeed(speed) {
    if (characteristic) {
        await sendCmd("s",speed)
    }
}
// Function to stop the train
async function stop() {
    halt = true;
    speedValue = 0;
    speedControl.value = 0;
    for (let i = 0; i < 60; i++) {
        if(await sendCmd("h",0)){
            halt = false;
            return;
        }
    }
    //await sendSpeed("s",0);  // Send 0 to stop the train
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
settingsButton.addEventListener('click', async () => {
    await sendCmd("a",0);
    settingsModal.style.display = 'flex';
});

// Close modal when 'x' is clicked
closeModal.addEventListener('click', () => {
    settingsModal.style.display = 'none';
});
light.onclick=()=>{
    sendCmd("l",Number(light.checked));
}


enhyst = document.getElementById("enhyst");
hysteresis = document.getElementById("hysteresis");
revertlock = document.getElementById("revertlock");
soundmotor = document.getElementById("soundmotor");
// Save settings and send to train
saveSettings= async () => {
    for (let i = 0; i < settingslist.length; i++) {
        const e = settingslist[i];
        if(e.type=="range"){
            await sendCmd("e",i+" "+Number(e.value));
        }
        else if(e.type=="checkbox"){
            await sendCmd("e",i+" "+Number(e.checked));
        }
    }
    // Close the modal after saving
    settingsModal.style.display = 'none';
}
saveSettingsButton.addEventListener('click', saveSettings);

// Close modal if user clicks outside the modal content
window.addEventListener('click', (event) => {
    if (event.target == settingsModal) {
        settingsModal.style.display = 'none';
    }
});
